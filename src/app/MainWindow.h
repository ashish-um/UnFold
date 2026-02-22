#pragma once

#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>

class SpatialView;
class SpatialScene;
class MiniMap;
class FilesystemWorker;
class FileWatcher;
class WorkspaceManager;

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
    void onNodeSelected(const QString &path);
    void onSelectionChanged();

private:
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    SpatialScene *m_scene;
    SpatialView *m_view;
    MiniMap *m_miniMap;
    FilesystemWorker *m_fsWorker;
    FileWatcher *m_fileWatcher;
    WorkspaceManager *m_workspaceManager;

    QLabel *m_statusLabel;
    QLabel *m_selectionLabel;
    QLabel *m_pathLabel;
};
