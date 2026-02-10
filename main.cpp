#include <QApplication>
#include <QGuiApplication>
#include <QIcon>

#include "bohmainwindow.h"
#include "chat_store.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // CRITICAL FOR WAYLAND (this fixes the W icon)
    QGuiApplication::setDesktopFileName("chatgpt-desktop");

    // Application metadata
    app.setApplicationName("ChatGPT Desktop");
    app.setOrganizationName("ai-sx");
    app.setOrganizationDomain("ai-sx.app");
    app.setApplicationVersion("1.2");

    // Set application icon (multiple attempts for Linux compatibility)
    QIcon appIcon(":/resources/icons/boh-chat.png");

    // Try setting at application level
    app.setWindowIcon(appIcon);

    // Also set as default for all windows
    QApplication::setWindowIcon(appIcon);

    // Initialize database
    ChatStore::init();

    BohMainWindow window;

    // Explicitly set icon on the main window too
    window.setWindowIcon(appIcon);

    window.show();

    return app.exec();
}
