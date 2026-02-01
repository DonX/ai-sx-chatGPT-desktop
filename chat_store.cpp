#include "chat_store.h"
#include "openai_client.h"  // For ChatMessage struct
#include <QtSql>
#include <QDir>

static QSqlDatabase db;

void ChatStore::init()
{
    if (QSqlDatabase::contains("chat"))
        return;

    db = QSqlDatabase::addDatabase("QSQLITE", "chat");
    db.setDatabaseName(QDir::homePath() + "/.ai-chatgpt.db");
    db.open();

    QSqlQuery q(db);

    // Threads table (extended, backward-safe)
    q.exec(
        "CREATE TABLE IF NOT EXISTS threads ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT,"
        "created DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated DATETIME DEFAULT CURRENT_TIMESTAMP)"
    );

    // Messages table (already thread-aware)
    q.exec(
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "thread_id INTEGER,"
        "role TEXT,"
        "content TEXT,"
        "ts DATETIME DEFAULT CURRENT_TIMESTAMP)"
    );
}

int ChatStore::ensureThread()
{
    QSqlQuery q(db);

    // Get most recently updated thread
    q.exec("SELECT id FROM threads ORDER BY updated DESC LIMIT 1");
    if (q.next())
        return q.value(0).toInt();

    // Create first thread
    q.exec("INSERT INTO threads DEFAULT VALUES");
    return q.lastInsertId().toInt();
}

int ChatStore::createThread(const QString &title)
{
    QSqlQuery q(db);

    q.prepare(
        "INSERT INTO threads(title) VALUES(?)"
    );
    q.addBindValue(title);
    q.exec();

    return q.lastInsertId().toInt();
}

static bool threadHasTitle(int threadId)
{
    QSqlQuery q(db);
    q.prepare("SELECT title FROM threads WHERE id=?");
    q.addBindValue(threadId);
    q.exec();

    if (q.next())
        return !q.value(0).toString().isEmpty();

    return false;
}

void ChatStore::addMessage(int threadId,
                           const QString &role,
                           const QString &content)
{
    QSqlQuery q(db);

    q.prepare("INSERT INTO messages(thread_id, role, content) VALUES(?,?,?)");
    q.addBindValue(threadId);
    q.addBindValue(role);
    q.addBindValue(content);
    q.exec();

    // Touch thread timestamp
    QSqlQuery t(db);
    t.prepare("UPDATE threads SET updated=CURRENT_TIMESTAMP WHERE id=?");
    t.addBindValue(threadId);
    t.exec();

    // STEP — AUTO SET THREAD TITLE (FIRST USER MESSAGE ONLY)
    if (role == "user" && !threadHasTitle(threadId)) {

        QString title = content;
        title.replace('\n', ' ');
        title = title.left(40).trimmed();

        setThreadTitle(threadId, title);
    }
}

void ChatStore::setThreadTitle(int threadId, const QString &title)
{
    QSqlQuery q(db);
    q.prepare("UPDATE threads SET title=? WHERE id=?");
    q.addBindValue(title);
    q.addBindValue(threadId);
    q.exec();
}
void ChatStore::renameThread(int threadId, const QString &title)
{
    // FIX: Added (db) parameter to use correct database connection
    QSqlQuery q(db);
    q.prepare("UPDATE threads SET title = ?, updated = CURRENT_TIMESTAMP WHERE id = ?");
    q.addBindValue(title);
    q.addBindValue(threadId);
    q.exec();
}

// =====================================================
// DELETE THREAD (and all associated messages)
// =====================================================
void ChatStore::deleteThread(int threadId)
{
    QSqlQuery q(db);

    // Delete all messages in this thread
    q.prepare("DELETE FROM messages WHERE thread_id = ?");
    q.addBindValue(threadId);
    q.exec();

    // Delete the thread itself
    q.prepare("DELETE FROM threads WHERE id = ?");
    q.addBindValue(threadId);
    q.exec();
}

QString ChatStore::loadThread(int threadId)
{
    QString out;
    QSqlQuery q(db);

    q.prepare(
        "SELECT role, content "
        "FROM messages "
        "WHERE thread_id=? "
        "ORDER BY id"
    );
    q.addBindValue(threadId);
    q.exec();

    while (q.next()) {
        QString role = q.value(0).toString();
        QString content = q.value(1).toString();

        // Map API roles to display names
        QString displayRole = role;
        if (role == "user") displayRole = "You";
        else if (role == "assistant") displayRole = "AI";

        out += "<b>" + displayRole + ":</b> " + content + "<br>";
    }
    return out;
}

// =====================================================
// LOAD THREAD MESSAGES (for API context)
// =====================================================
QList<ChatMessage> ChatStore::loadThreadMessages(int threadId)
{
    QList<ChatMessage> messages;
    QSqlQuery q(db);

    q.prepare(
        "SELECT role, content "
        "FROM messages "
        "WHERE thread_id=? "
        "ORDER BY id"
    );
    q.addBindValue(threadId);
    q.exec();

    while (q.next()) {
        ChatMessage msg;
        QString role = q.value(0).toString();

        // Map display roles to API roles
        if (role == "You" || role == "user") {
            msg.role = "user";
        } else if (role == "AI" || role == "assistant") {
            msg.role = "assistant";
        } else {
            msg.role = role; // system, etc.
        }

        msg.content = q.value(1).toString();
        messages.append(msg);
    }

    return messages;
}

QList<ChatThread> ChatStore::loadThreads()
{
    QList<ChatThread> threads;
    QSqlQuery q(db);

    q.exec(
        "SELECT id, title, updated "
        "FROM threads "
        "ORDER BY updated DESC"
    );

    while (q.next()) {
        ChatThread t;
        t.id = q.value(0).toInt();
        t.title = q.value(1).toString().isEmpty()
                  ? QString("Chat %1").arg(t.id)
                  : q.value(1).toString();
        t.updated = q.value(2).toString();
        threads.append(t);
    }

    return threads;
}
