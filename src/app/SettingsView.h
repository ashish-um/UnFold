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
    void loadWorkspaceRequested();

private slots:
    void onBrowseDirectory();
    void onSaveAndBack();
    void onLoadWorkspace();

private:
    QCheckBox *m_hiddenItemsCheck;
    QLineEdit *m_defaultDirEdit;
    QPushButton *m_browseButton;
    QPushButton *m_loadWorkspaceBtn;
};
