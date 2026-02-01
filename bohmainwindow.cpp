#include "bohmainwindow.h"
#include "ui_bohmainwindow.h"

#include "openai_client.h"
#include "chat_store.h"
#include "settingsdialog.h"

#include <QScrollBar>
#include <QKeyEvent>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>

// =====================================================
// Constructor
// =====================================================
BohMainWindow::BohMainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::BohMainWindow)
{
    // Build UI from .ui file
    ui->setupUi(this);

    // -------------------------------------------------
    // Window / Application Identity
    // -------------------------------------------------
    // Set explicit window icon (prevents OS fallback icon like orange "W")
    setWindowIcon(QIcon(":/resources/icons/boh-chat.png"));

    // -------------------------------------------------
    // Preferences dialog
    // -------------------------------------------------
    connect(ui->actionPreferences, &QAction::triggered,
            this, [this]() {
        SettingsDialog dlg(this);
        dlg.exec();
    });

    // =================================================
    // THREAD LIST — VISUAL & BEHAVIOR SETUP (ONE TIME)
    // =================================================
    // These settings control row height, spacing,
    // hover/selection behavior, and scrolling smoothness.
    ui->threadsList->setSpacing(0);
    ui->threadsList->setUniformItemSizes(true);
    ui->threadsList->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel
    );

    // Enable context menu (right-click)
    ui->threadsList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->threadsList, &QListWidget::customContextMenuRequested,
            this, &BohMainWindow::showThreadContextMenu);

    ui->threadsList->setStyleSheet(R"(
        QListWidget {
            border: none;
            background: #1e1e1e;
        }

        QListWidget::item {
            height: 28px;        /* controls vertical spacing */
            padding: 2px 8px;    /* top/bottom | left/right */
            margin: 0px;
            color: #dddddd;
        }

        QListWidget::item:hover {
            background-color: #2b5fb8;
        }

        QListWidget::item:selected {
            background-color: #2b5fb8;
            color: white;
        }
    )");

    // =================================================
    // STEP 1 — LOAD THREADS INTO threadsList
    // =================================================
    currentThreadId = ChatStore::ensureThread();
    reloadThreadsList();

    // =================================================
    // STEP 2 — LOAD CURRENT THREAD CONTENT
    // =================================================
    ui->chatHistory->setHtml(
        ChatStore::loadThread(currentThreadId)
    );

    // =================================================
    // STEP 3 — THREAD SELECTION → LOAD CHAT
    // =================================================
    connect(ui->threadsList, &QListWidget::itemClicked,
            this, [this](QListWidgetItem *item) {

        int threadId = item->data(Qt::UserRole).toInt();
        currentThreadId = threadId;

        ui->chatHistory->setHtml(
            ChatStore::loadThread(threadId)
        );
    });

    // =================================================
    // STEP 4 — NEW CHAT BUTTON
    // =================================================
    connect(ui->newChatBtn, &QPushButton::clicked,
            this, [this]() {

        // Create new thread in DB
        currentThreadId = ChatStore::createThread();

        // Clear chat display
        ui->chatHistory->clear();

        // Reload thread list and select new thread
        reloadThreadsList();
    });

    // =================================================
    // LEFT PANEL VISUAL SEPARATION
    // =================================================
    ui->leftPanel->setStyleSheet(
        "QWidget {"
        " border-right: 1px solid #444;"
        " background: #1e1e1e;"
        "}"
    );

    // =================================================
    // INPUT & BUTTON AFFORDANCES
    // =================================================
    ui->userInput->setPlaceholderText("Ask @i…");
    ui->newChatBtn->setText("＋ New Chat");

    // =================================================
    // OpenAI Client Wiring
    // =================================================
    openai = new OpenAIClient(this);

    connect(ui->sendButton, &QPushButton::clicked,
            this, &BohMainWindow::onSendClicked);

    connect(openai, &OpenAIClient::replyReady, this,
            [this](const QString &reply) {

        ui->chatHistory->append("<b>AI:</b> " + reply);
        ui->chatHistory->verticalScrollBar()->setValue(
            ui->chatHistory->verticalScrollBar()->maximum()
        );

        ChatStore::addMessage(
            currentThreadId, "assistant", reply
        );

        ui->sendButton->setEnabled(true);
        ui->sendButton->setText("Send ➤");
        ui->statusbar->showMessage("Response received", 3000);
    });

    connect(openai, &OpenAIClient::errorOccurred, this,
            [this](const QString &err) {

        ui->chatHistory->append("<b style='color: #ff6b6b;'>Error:</b> " + err);
        ui->chatHistory->verticalScrollBar()->setValue(
            ui->chatHistory->verticalScrollBar()->maximum()
        );

        ui->sendButton->setEnabled(true);
        ui->sendButton->setText("Send ➤");
        ui->statusbar->showMessage("Error: " + err, 5000);
    });
}

// =====================================================
// Destructor
// =====================================================
BohMainWindow::~BohMainWindow()
{
    delete ui;
}

// =====================================================
// Send Message (with conversation context)
// =====================================================
void BohMainWindow::onSendClicked()
{
    const QString text =
        ui->userInput->toPlainText().trimmed();

    if (text.isEmpty())
        return;

    ui->chatHistory->append("<b>You:</b> " + text);
    ui->chatHistory->verticalScrollBar()->setValue(
        ui->chatHistory->verticalScrollBar()->maximum()
    );

    ChatStore::addMessage(
        currentThreadId, "user", text
    );

    ui->userInput->clear();
    ui->sendButton->setEnabled(false);
    ui->sendButton->setText("Sending...");

    // Show status message
    ui->statusbar->showMessage("Sending message to OpenAI...");

    // =====================================================
    // IMPORTANT: Load conversation history for context
    // =====================================================
    auto history = ChatStore::loadThreadMessages(currentThreadId);
    openai->sendMessage(text, history);
}

// =====================================================
// Keyboard Handling (Enter / Shift+Enter)
// =====================================================
void BohMainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {

        if (event->modifiers() & Qt::ShiftModifier) {
            ui->userInput->insertPlainText("\n");
        } else {
            onSendClicked();
        }
        return;
    }

    QMainWindow::keyPressEvent(event);
}

// =====================================================
// CONTEXT MENU for Thread List (Right-Click)
// =====================================================
void BohMainWindow::showThreadContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = ui->threadsList->itemAt(pos);
    if (!item)
        return;

    int threadId = item->data(Qt::UserRole).toInt();
    QString threadTitle = item->text();

    QMenu menu(this);

    // Rename action
    QAction *renameAction = menu.addAction("✏️ Rename");

    // Delete action
    QAction *deleteAction = menu.addAction("🗑️ Delete");

    // Show menu and get selected action
    QAction *selected = menu.exec(ui->threadsList->mapToGlobal(pos));

    if (selected == renameAction) {
        // Make item editable and enter edit mode
        ui->threadsList->editItem(item);
    }
    else if (selected == deleteAction) {
        // Confirm deletion
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Delete Thread",
            QString("Are you sure you want to delete '%1'?\n\nThis will permanently delete all messages in this conversation.").arg(threadTitle),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            // Delete from database
            ChatStore::deleteThread(threadId);

            // If we're deleting the current thread, switch to another one
            if (threadId == currentThreadId) {
                // Get remaining threads
                auto remainingThreads = ChatStore::loadThreads();

                if (remainingThreads.isEmpty()) {
                    // No threads left - create a new one
                    currentThreadId = ChatStore::createThread();
                    ui->chatHistory->clear();
                } else {
                    // Switch to the first remaining thread
                    currentThreadId = remainingThreads.first().id;
                    ui->chatHistory->setHtml(
                        ChatStore::loadThread(currentThreadId)
                    );
                }
            }

            // Reload thread list
            reloadThreadsList();
        }
    }
}

// =====================================================
// RELOAD THREADS LIST (Helper Function)
// =====================================================
void BohMainWindow::reloadThreadsList()
{
    ui->threadsList->clear();
    auto threads = ChatStore::loadThreads();

    for (const auto &t : threads) {
        auto *item = new QListWidgetItem(
            t.title.isEmpty() ? "New Chat" : t.title,
            ui->threadsList
        );

        item->setData(Qt::UserRole, t.id);
        item->setSizeHint(QSize(0, 28));

        // Enable inline rename (double-click)
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        // Timestamp tooltip (hover)
        item->setToolTip("Last updated: " + t.updated);

        // Select current thread
        if (t.id == currentThreadId) {
            ui->threadsList->setCurrentItem(item);
        }
    }

    // Wire up rename handler (only once per reload)
    disconnect(ui->threadsList, &QListWidget::itemChanged, nullptr, nullptr);
    connect(ui->threadsList, &QListWidget::itemChanged,
            this, [](QListWidgetItem *item) {
        int threadId = item->data(Qt::UserRole).toInt();
        QString title = item->text().trimmed();

        if (!title.isEmpty()) {
            ChatStore::renameThread(threadId, title);
        }
    });
}
