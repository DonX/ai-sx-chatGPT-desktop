#ifndef BOHMAINWINDOW_H
#define BOHMAINWINDOW_H

#include <QMainWindow>
#include "openai_client.h"

class QSplitter;
class QListWidget;
class QTextBrowser;
class QPlainTextEdit;
class QPushButton;
class QWidget;

namespace Ui {
class BohMainWindow;
}

class BohMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BohMainWindow(QWidget *parent = nullptr);
    ~BohMainWindow();

private:
    Ui::BohMainWindow *ui;
    int currentThreadId;
    OpenAIClient *openai;

    // ----- Responsive layout members -----
    QSplitter   *mainSplitter;
    QWidget     *sidebarWidget;
    QPushButton *toggleSidebarBtn;
    QListWidget *threadsList;
    QPushButton *newChatBtn;
    QTextBrowser *chatHistory;
    QPlainTextEdit *userInput;
    QPushButton *sendButton;
    int          savedSidebarWidth;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSendClicked();
    void showThreadContextMenu(const QPoint &pos);
    void reloadThreadsList();
    void toggleSidebar();
};

#endif // BOHMAINWINDOW_H
