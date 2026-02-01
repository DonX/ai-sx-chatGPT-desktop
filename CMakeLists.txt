cmake_minimum_required(VERSION 3.16)
project(boh_chat_desktop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Sql)

add_executable(boh_chat_desktop
  main.cpp
  bohmainwindow.h bohmainwindow.cpp bohmainwindow.ui
  openai_client.h openai_client.cpp
  chat_store.h chat_store.cpp
  settingsdialog.h
  settingsdialog.cpp
  settingsdialog.ui
  app_icons.qrc
)

target_link_libraries(boh_chat_desktop
  Qt5::Widgets
  Qt5::Network
  Qt5::Sql
)
