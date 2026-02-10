cmake_minimum_required(VERSION 3.16)
project(boh_chat_desktop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Sql)

add_executable(chatgpt-desktop
  main.cpp
  bohmainwindow.h bohmainwindow.cpp bohmainwindow.ui
  openai_client.h openai_client.cpp
  chat_store.h chat_store.cpp
  markdown_render.h markdown_render.cpp
  settingsdialog.h
  settingsdialog.cpp
  settingsdialog.ui
  app_icons.qrc
)

target_link_libraries(chatgpt-desktop
  Qt5::Widgets
  Qt5::Network
  Qt5::Sql
)

# ----------------------------
# Install rules (REQUIRED for RPM / OBS)
# ----------------------------

# Install the application binary
install(TARGETS chatgpt-desktop
    RUNTIME DESTINATION bin
)

# Install desktop entry
install(FILES chatgpt-desktop.desktop
    DESTINATION share/applications
)

# Install application icon
install(FILES resources/icons/boh-chat.png
    DESTINATION share/icons/hicolor/128x128/apps
    RENAME chatgpt-desktop.png
)
