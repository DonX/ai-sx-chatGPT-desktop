#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#pragma once

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private:
    Ui::SettingsDialog *ui;

    void loadSettings();
    void saveSettings();
};

#endif // SETTINGSDIALOG_H
