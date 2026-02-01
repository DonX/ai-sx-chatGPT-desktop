#pragma once

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>


struct ChatMessage
{
    QString role;    // "user", "assistant", or "system"
    QString content;
};

class OpenAIClient : public QObject
{
    Q_OBJECT
public:
    explicit OpenAIClient(QObject *parent = nullptr);

    // Send message with full conversation context
    void sendMessage(const QString &text, const QList<ChatMessage> &history = QList<ChatMessage>());

signals:
    void replyReady(const QString &reply);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager manager;
};
