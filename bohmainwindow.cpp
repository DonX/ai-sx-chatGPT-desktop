#include "bohmainwindow.h"
#include "ui_bohmainwindow.h"

#include "openai_client.h"
#include "chat_store.h"
#include "settingsdialog.h"
#include "markdown_render.h"

#include <QScrollBar>
#include <QKeyEvent>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextBrowser>
#include <QPlainTextEdit>
#include <QPushButton>

// =====================================================
// Constructor
// =====================================================
BohMainWindow::BohMainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::BohMainWindow),
      savedSidebarWidth(220)
{
    // Build UI skeleton from .ui file (menu, statusbar, central layout)
    ui->setupUi(this);

    // -------------------------------------------------
    // Window / Application Identity
    // -------------------------------------------------
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
    // BUILD RESPONSIVE LAYOUT (programmatic)
    // =================================================

    // --- Sidebar: toggle button + "New Chat" + thread list ---
    sidebarWidget = new QWidget(this);
    sidebarWidget->setMinimumWidth(180);
    sidebarWidget->setMaximumWidth(320);
    sidebarWidget->setStyleSheet(
        "QWidget#sidebarWidget {"
        " border-right: 1px solid #444;"
        " background: #1e1e1e;"
        "}"
    );
    sidebarWidget->setObjectName("sidebarWidget");

    auto *sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(4, 4, 4, 4);
    sidebarLayout->setSpacing(4);

    newChatBtn = new QPushButton("＋ New Chat", sidebarWidget);
    newChatBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #2b5fb8;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #3a6fc8; }
        QPushButton:pressed { background-color: #1a4f98; }
    )");

    threadsList = new QListWidget(sidebarWidget);

    sidebarLayout->addWidget(newChatBtn);
    sidebarLayout->addWidget(threadsList, 1);  // stretch=1 fills remaining space

    // --- Right side: chat area + input area ---
    auto *chatPanel = new QWidget(this);
    auto *chatPanelLayout = new QVBoxLayout(chatPanel);
    chatPanelLayout->setContentsMargins(0, 0, 0, 0);
    chatPanelLayout->setSpacing(0);

    // Toggle sidebar button (sits above chat area)
    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(4, 4, 4, 0);

    toggleSidebarBtn = new QPushButton("☰", this);
    toggleSidebarBtn->setFixedSize(32, 28);
    toggleSidebarBtn->setToolTip("Toggle sidebar");
    toggleSidebarBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #aaa;
            border: 1px solid #444;
            border-radius: 4px;
            font-size: 16px;
        }
        QPushButton:hover { background: #333; color: white; }
    )");
    topBar->addWidget(toggleSidebarBtn);
    topBar->addStretch(1);

    chatPanelLayout->addLayout(topBar);

    // Chat history (stretches to fill)
    chatHistory = new QTextBrowser(chatPanel);
    chatPanelLayout->addWidget(chatHistory, 1);  // stretch=1

    // Input area (fixed proportion at bottom)
    auto *inputWidget = new QWidget(chatPanel);
    auto *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(4, 4, 4, 4);
    inputLayout->setSpacing(4);

    userInput = new QPlainTextEdit(inputWidget);
    userInput->setPlaceholderText("Ask @i…");
    userInput->setMaximumHeight(120);

    sendButton = new QPushButton("Send ➤", inputWidget);
    sendButton->setMinimumSize(80, 40);
    sendButton->setStyleSheet(R"(
        QPushButton {
            background-color: #2b5fb8;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover { background-color: #3a6fc8; }
        QPushButton:pressed { background-color: #1a4f98; }
        QPushButton:disabled { background-color: #555; color: #999; }
    )");

    inputLayout->addWidget(userInput, 1);
    inputLayout->addWidget(sendButton);

    chatPanelLayout->addWidget(inputWidget);

    // --- QSplitter: sidebar | chat panel ---
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(sidebarWidget);
    mainSplitter->addWidget(chatPanel);
    mainSplitter->setStretchFactor(0, 0);  // sidebar: don't auto-stretch
    mainSplitter->setStretchFactor(1, 1);  // chat: takes all extra space
    mainSplitter->setSizes({220, 860});
    mainSplitter->setChildrenCollapsible(false);
    mainSplitter->setHandleWidth(2);

    // Add splitter to the central widget layout from .ui
    ui->centralwidget->layout()->addWidget(mainSplitter);

    // =================================================
    // SIDEBAR TOGGLE (collapsible drawer)
    // =================================================
    connect(toggleSidebarBtn, &QPushButton::clicked,
            this, &BohMainWindow::toggleSidebar);

    // =================================================
    // THREAD LIST — VISUAL & BEHAVIOR SETUP (ONE TIME)
    // =================================================
    threadsList->setSpacing(0);
    threadsList->setUniformItemSizes(true);
    threadsList->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel
    );

    // Enable context menu (right-click)
    threadsList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(threadsList, &QListWidget::customContextMenuRequested,
            this, &BohMainWindow::showThreadContextMenu);

    threadsList->setStyleSheet(R"(
        QListWidget {
            border: none;
            background: #1e1e1e;
        }

        QListWidget::item {
            height: 28px;
            padding: 2px 8px;
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
    chatHistory->setHtml(
        ChatStore::loadThread(currentThreadId)
    );

    // =================================================
    // STEP 3 — THREAD SELECTION → LOAD CHAT
    // =================================================
    connect(threadsList, &QListWidget::itemClicked,
            this, [this](QListWidgetItem *item) {

        int threadId = item->data(Qt::UserRole).toInt();
        currentThreadId = threadId;

        chatHistory->setHtml(
            ChatStore::loadThread(threadId)
        );
    });

    // =================================================
    // STEP 4 — NEW CHAT BUTTON
    // =================================================
    connect(newChatBtn, &QPushButton::clicked,
            this, [this]() {

        // Create new thread in DB
        currentThreadId = ChatStore::createThread();

        // Clear chat display
        chatHistory->clear();

        // Reload thread list and select new thread
        reloadThreadsList();
    });

    // =================================================
    // OpenAI Client Wiring
    // =================================================
    openai = new OpenAIClient(this);

    connect(sendButton, &QPushButton::clicked,
            this, &BohMainWindow::onSendClicked);

    connect(openai, &OpenAIClient::replyReady, this,
            [this](const QString &reply) {

        chatHistory->append("<b>AI:</b><br>" + MarkdownRender::toHtml(reply));
        chatHistory->verticalScrollBar()->setValue(
            chatHistory->verticalScrollBar()->maximum()
        );

        ChatStore::addMessage(
            currentThreadId, "assistant", reply
        );

        sendButton->setEnabled(true);
        sendButton->setText("Send ➤");
        ui->statusbar->showMessage("Response received", 3000);
    });

    connect(openai, &OpenAIClient::errorOccurred, this,
            [this](const QString &err) {

        chatHistory->append("<b style='color: #ff6b6b;'>Error:</b> " + err);
        chatHistory->verticalScrollBar()->setValue(
            chatHistory->verticalScrollBar()->maximum()
        );

        sendButton->setEnabled(true);
        sendButton->setText("Send ➤");
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
        userInput->toPlainText().trimmed();

    if (text.isEmpty())
        return;

    chatHistory->append("<b>You:</b> " + text.toHtmlEscaped().replace("\n", "<br>"));
    chatHistory->verticalScrollBar()->setValue(
        chatHistory->verticalScrollBar()->maximum()
    );

    ChatStore::addMessage(
        currentThreadId, "user", text
    );

    userInput->clear();
    sendButton->setEnabled(false);
    sendButton->setText("Sending...");

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
            userInput->insertPlainText("\n");
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
    QListWidgetItem *item = threadsList->itemAt(pos);
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
    QAction *selected = menu.exec(threadsList->mapToGlobal(pos));

    if (selected == renameAction) {
        // Make item editable and enter edit mode
        threadsList->editItem(item);
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
                    chatHistory->clear();
                } else {
                    // Switch to the first remaining thread
                    currentThreadId = remainingThreads.first().id;
                    chatHistory->setHtml(
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
// SIDEBAR TOGGLE (collapsible drawer)
// =====================================================
void BohMainWindow::toggleSidebar()
{
    if (sidebarWidget->isVisible()) {
        // Save current width before hiding
        savedSidebarWidth = mainSplitter->sizes().at(0);
        sidebarWidget->hide();
        toggleSidebarBtn->setText("☰");
        toggleSidebarBtn->setToolTip("Show sidebar");
    } else {
        sidebarWidget->show();
        mainSplitter->setSizes({savedSidebarWidth,
                                width() - savedSidebarWidth});
        toggleSidebarBtn->setText("✕");
        toggleSidebarBtn->setToolTip("Hide sidebar");
    }
}

// =====================================================
// RELOAD THREADS LIST (Helper Function)
// =====================================================
void BohMainWindow::reloadThreadsList()
{
    threadsList->clear();
    auto threads = ChatStore::loadThreads();

    for (const auto &t : threads) {
        auto *item = new QListWidgetItem(
            t.title.isEmpty() ? "New Chat" : t.title,
            threadsList
        );

        item->setData(Qt::UserRole, t.id);
        item->setSizeHint(QSize(0, 28));

        // Enable inline rename (double-click)
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        // Timestamp tooltip (hover)
        item->setToolTip("Last updated: " + t.updated);

        // Select current thread
        if (t.id == currentThreadId) {
            threadsList->setCurrentItem(item);
        }
    }

    // Wire up rename handler (only once per reload)
    disconnect(threadsList, &QListWidget::itemChanged, nullptr, nullptr);
    connect(threadsList, &QListWidget::itemChanged,
            this, [](QListWidgetItem *item) {
        int threadId = item->data(Qt::UserRole).toInt();
        QString title = item->text().trimmed();

        if (!title.isEmpty()) {
            ChatStore::renameThread(threadId, title);
        }
    });
}
