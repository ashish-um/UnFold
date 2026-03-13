#pragma once

#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QStackedWidget>

class SpatialView;
class SpatialScene;
class MiniMap;
class FilesystemWorker;
class FileWatcher;
class WorkspaceManager;
class SettingsView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNavigateHome();
    void onSaveWorkspace();
    void onLoadWorkspace();
    void onCollapseAll();
    void onOpenSettings();
    void onCloseSettings();
    void onNodeSelected(const QString &path);
    void onSelectionChanged();

private:
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    QStackedWidget *m_stackedWidget;
    SpatialScene *m_scene;
    SpatialView *m_view;
    SettingsView *m_settingsView;
    MiniMap *m_miniMap;
    FilesystemWorker *m_fsWorker;
    FileWatcher *m_fileWatcher;
    WorkspaceManager *m_workspaceManager;

    QLabel *m_statusLabel;
    QLabel *m_selectionLabel;
    QLabel *m_pathLabel;
};
