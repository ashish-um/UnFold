#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

class SettingsView : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsView(QWidget *parent = nullptr);

signals:
    void backRequested();

private slots:
    void onBrowseDirectory();
    void onSaveAndBack();

private:
    QCheckBox *m_hiddenItemsCheck;
    QLineEdit *m_defaultDirEdit;
    QPushButton *m_browseButton;
};
