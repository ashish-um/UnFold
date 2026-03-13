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
    void onLoadWorkspace();
    void onCollapseAll();
    void onOpenSettings();
    void onCloseSettings();
    void onNodeSelected(const QString &path);
    void onSelectionChanged();
    void onAutoSaveTimeout();

private:
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    QStackedWidget *m_stackedWidget = nullptr;
    SpatialScene *m_scene = nullptr;
    SpatialView *m_view = nullptr;
    SettingsView *m_settingsView = nullptr;
    MiniMap *m_miniMap = nullptr;
    FilesystemWorker *m_fsWorker = nullptr;
    FileWatcher *m_fileWatcher = nullptr;
    WorkspaceManager *m_workspaceManager = nullptr;

    QLabel *m_statusLabel = nullptr;
    QLabel *m_selectionLabel = nullptr;
    QLabel *m_pathLabel = nullptr;

    QTimer *m_autoSaveTimer = nullptr;
};
