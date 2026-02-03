#ifndef BOHMAINWINDOW_H
#define BOHMAINWINDOW_H

#include <QMainWindow>
#include "openai_client.h"

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

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSendClicked();
    void showThreadContextMenu(const QPoint &pos);
    void reloadThreadsList();
};

#endif // BOHMAINWINDOW_H
