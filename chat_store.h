#pragma once

#include <QString>
#include <QList>

// Forward declaration (defined in openai_client.h)
struct ChatMessage;

struct ChatThread
{
    int id;
    QString title;
    QString updated;
};

class ChatStore
{
public:
    static void init();

    static int ensureThread();
    static int createThread(const QString &title = QString());

    static void addMessage(int threadId, const QString &role, const QString &content);
    static QString loadThread(int threadId);
    static QList<ChatMessage> loadThreadMessages(int threadId);
    static void renameThread(int threadId, const QString &title);
    static void deleteThread(int threadId);

    static QList<ChatThread> loadThreads();
    static void setThreadTitle(int threadId, const QString &title);
};
