#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QComboBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Make API key hidden by default
    ui->apiKeyEdit->setEchoMode(QLineEdit::Password);

    loadSettings();

    // OK saves, Cancel does nothing
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        saveSettings();
        accept();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        reject();
    });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadSettings()
{
    QSettings s;

    ui->apiKeyEdit->setText(s.value("openai/api_key").toString());

    // =====================================================
    // UPDATED MODEL LIST (Feb 2025)
    // =====================================================
    ui->modelCombo->clear();

    // GPT-4o Series (Recommended - Best Performance)
    ui->modelCombo->addItem("gpt-4o (Latest flagship, 128K context)");
    ui->modelCombo->addItem("gpt-4o-mini (Faster & cheaper, 128K context)");

    // o1 Series (Advanced Reasoning)
    ui->modelCombo->addItem("o1 (Best reasoning, math, coding)");
    ui->modelCombo->addItem("o1-mini (Faster reasoning)");

    // GPT-4 Turbo Series
    ui->modelCombo->addItem("gpt-4-turbo (Previous gen, 128K context)");

    // GPT-3.5 Turbo (Budget-Friendly)
    ui->modelCombo->addItem("gpt-3.5-turbo (Fast & cheap, 16K context)");

    // Load saved model (extract model ID from display text)
    QString savedModel = s.value("openai/model", "gpt-4o-mini").toString();

    // Find matching item (search for model ID in display text)
    int idx = -1;
    for (int i = 0; i < ui->modelCombo->count(); ++i) {
        if (ui->modelCombo->itemText(i).startsWith(savedModel)) {
            idx = i;
            break;
        }
    }

    if (idx >= 0)
        ui->modelCombo->setCurrentIndex(idx);
    else
        ui->modelCombo->setCurrentIndex(1); // Default to gpt-4o-mini
}

void SettingsDialog::saveSettings()
{
    QSettings s;

    s.setValue("openai/api_key", ui->apiKeyEdit->text().trimmed());

    // Extract model ID from display text (e.g., "gpt-4o (Latest...)" → "gpt-4o")
    QString displayText = ui->modelCombo->currentText();
    QString modelId = displayText.split(" ").first();

    s.setValue("openai/model", modelId);
}
