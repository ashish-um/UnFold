#include "SettingsView.h"
#include "persistence/SettingsManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QLabel>

SettingsView::SettingsView(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Settings");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #fff;");
    mainLayout->addWidget(titleLabel);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(15);
    
    m_hiddenItemsCheck = new QCheckBox("Show hidden files and folders");
    m_hiddenItemsCheck->setChecked(SettingsManager::instance().showHiddenItems());
    formLayout->addRow("Filesystem:", m_hiddenItemsCheck);

    QHBoxLayout *dirLayout = new QHBoxLayout();
    m_defaultDirEdit = new QLineEdit(SettingsManager::instance().defaultDirectory());
    m_browseButton = new QPushButton("Browse...");
    dirLayout->addWidget(m_defaultDirEdit);
    dirLayout->addWidget(m_browseButton);
    formLayout->addRow("Start Directory:", dirLayout);

    mainLayout->addLayout(formLayout);

    mainLayout->addStretch(1);

    QPushButton *saveBtn = new QPushButton("← Save & Return");
    saveBtn->setMinimumWidth(150);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(saveBtn);
    btnLayout->addStretch(1);
    
    mainLayout->addLayout(btnLayout);

    connect(m_browseButton, &QPushButton::clicked, this, &SettingsView::onBrowseDirectory);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsView::onSaveAndBack);

    // Styling
    setStyleSheet(
        "QWidget { background: #222226; color: #ddd; font-size: 14px; }"
        "QLabel { color: #ddd; }"
        "QCheckBox { color: #ddd; }"
        "QLineEdit { background: #333; border: 1px solid #555; color: #ddd; padding: 6px; border-radius: 4px; }"
        "QPushButton { background: #3a3a3a; border: 1px solid #555; color: #ddd; padding: 8px 16px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background: #4a4a4a; border-color: #7ab3e0; }"
    );
}

void SettingsView::onBrowseDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Default Directory",
                                                   m_defaultDirEdit->text());
    if (!dir.isEmpty()) {
        m_defaultDirEdit->setText(dir);
    }
}

void SettingsView::onSaveAndBack()
{
    SettingsManager::instance().setShowHiddenItems(m_hiddenItemsCheck->isChecked());
    SettingsManager::instance().setDefaultDirectory(m_defaultDirEdit->text());
    emit backRequested();
}
