#include "openai_client.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

// =====================================================
// NOTE: API key is now loaded from QSettings
// User must configure it via Settings → Preferences
// =====================================================

OpenAIClient::OpenAIClient(QObject *parent)
    : QObject(parent)
{
}

void OpenAIClient::sendMessage(const QString &text, const QList<ChatMessage> &history)
{
    // =====================================================
    // STEP 1: Load API key and model from user settings
    // =====================================================
    QSettings settings;
    QString apiKey = settings.value("openai/api_key").toString();
    QString model = settings.value("openai/model", "gpt-4o-mini").toString();

    // Validate API key exists
    if (apiKey.isEmpty()) {
        emit errorOccurred("⚠️ API key not configured.\n\nPlease set your OpenAI API key in:\nSettings → Preferences");
        return;
    }

    // =====================================================
    // STEP 2: Build API request with conversation history
    // =====================================================
    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
        QByteArray("Bearer ") + apiKey.toUtf8());

    // Build messages array with full conversation history
    QJsonArray messages;

    // Add conversation history (if provided)
    for (const auto &msg : history) {
        QJsonObject historyMsg;
        historyMsg["role"] = msg.role;
        historyMsg["content"] = msg.content;
        messages.append(historyMsg);
    }

    // Add current user message
    QJsonObject currentMessage;
    currentMessage["role"] = "user";
    currentMessage["content"] = text;
    messages.append(currentMessage);

    // Build request body
    QJsonObject body;
    body["model"] = model;
    body["messages"] = messages;

    // =====================================================
    // STEP 3: Send request and handle response
    // =====================================================
    auto reply = manager.post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Handle network errors
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("Network Error: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        // Parse JSON response
        auto json = QJsonDocument::fromJson(reply->readAll()).object();

        // Check for API errors (invalid key, quota exceeded, etc.)
        if (json.contains("error")) {
            QString errorMsg = json["error"].toObject()["message"].toString();
            emit errorOccurred("OpenAI API Error:\n" + errorMsg);
            reply->deleteLater();
            return;
        }

        // Extract AI response
        auto content =
            json["choices"].toArray()[0].toObject()
            ["message"].toObject()["content"].toString();

        emit replyReady(content);
        reply->deleteLater();
    });
}
