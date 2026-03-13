#include "MainWindow.h"
#include "MiniMap.h"
#include "canvas/SpatialView.h"
#include "canvas/SpatialScene.h"
#include "filesystem/FilesystemWorker.h"
#include "filesystem/FileWatcher.h"
#include "persistence/WorkspaceManager.h"
#include "items/NodeItem.h"
#include "persistence/SettingsManager.h"
#include "app/SettingsView.h"
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Unfold");
    setMinimumSize(1024, 768);

    // Create the filesystem worker (runs on its own thread)
    m_fsWorker = new FilesystemWorker(this);

    // Create the file watcher
    m_fileWatcher = new FileWatcher(this);

    // Create the scene and view
    m_scene = new SpatialScene(m_fsWorker, m_fileWatcher, this);
    m_view = new SpatialView(m_scene, this);

    // Create the stacked widget and setting view
    m_stackedWidget = new QStackedWidget(this);
    m_settingsView = new SettingsView(this);
    m_stackedWidget->addWidget(m_view);
    m_stackedWidget->addWidget(m_settingsView);

    setCentralWidget(m_stackedWidget);

    // Create the minimap overlay
    m_miniMap = new MiniMap(m_scene, m_view, m_view);

    // Create workspace manager
    m_workspaceManager = new WorkspaceManager(this);

    setupToolBar();
    setupStatusBar();
    setupConnections();

    // Spawn initial drive nodes (or start directory)
    m_scene->spawnDriveNodes();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_workspaceManager->autoSave(m_scene, m_view);
    event->accept();
}

void MainWindow::setupToolBar()
{
    QToolBar *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setStyleSheet(
        "QToolBar { background: #252525; border-bottom: 1px solid #3a3a3a; padding: 4px; spacing: 6px; }"
        "QToolButton { background: #353535; border: 1px solid #4a4a4a; border-radius: 4px; padding: 6px 12px; color: #ddd; font-size: 12px; }"
        "QToolButton:hover { background: #454545; border-color: #5a8abf; }"
        "QToolButton:pressed { background: #2a5a8a; }"
    );

    QAction *homeAction = toolbar->addAction("⌂ Home");
    homeAction->setToolTip("Navigate to drives root");
    connect(homeAction, &QAction::triggered, this, &MainWindow::onNavigateHome);

    toolbar->addSeparator();

    QAction *saveAction = toolbar->addAction("💾 Save");
    saveAction->setToolTip("Save current workspace");
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveWorkspace);

    QAction *loadAction = toolbar->addAction("📂 Load");
    loadAction->setToolTip("Load a saved workspace");
    connect(loadAction, &QAction::triggered, this, &MainWindow::onLoadWorkspace);

    toolbar->addSeparator();

    QAction *collapseAction = toolbar->addAction("⊟ Collapse All");
    collapseAction->setToolTip("Collapse all expanded nodes");
    connect(collapseAction, &QAction::triggered, this, &MainWindow::onCollapseAll);

    toolbar->addSeparator();

    QAction *settingsAction = toolbar->addAction("⚙ Settings");
    settingsAction->setToolTip("Open settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    m_selectionLabel = new QLabel("");
    m_pathLabel = new QLabel("");

    statusBar()->setStyleSheet(
        "QStatusBar { background: #252525; border-top: 1px solid #3a3a3a; color: #aaa; }"
        "QLabel { color: #aaa; padding: 2px 8px; }"
    );
    m_selectionLabel->setStyleSheet("QLabel { color: #7ab3e0; font-weight: 500; }");

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_selectionLabel);
    statusBar()->addPermanentWidget(m_pathLabel);
}

void MainWindow::setupConnections()
{
    connect(m_scene, &SpatialScene::nodeSelected, this, &MainWindow::onNodeSelected);
    connect(m_scene, &SpatialScene::statusMessage, m_statusLabel, &QLabel::setText);
    connect(m_scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_settingsView, &SettingsView::backRequested, this, &MainWindow::onCloseSettings);
}

void MainWindow::onNavigateHome()
{
    m_scene->resetToHome();
    m_view->resetView();
}

void MainWindow::onSaveWorkspace()
{
    QString path = QFileDialog::getSaveFileName(this, "Save Workspace",
        QString(), "Spatial Workspace (*.spatial)");
    if (!path.isEmpty()) {
        m_workspaceManager->save(path, m_scene, m_view);
        m_statusLabel->setText("Workspace saved.");
    }
}

void MainWindow::onLoadWorkspace()
{
    QString path = QFileDialog::getOpenFileName(this, "Load Workspace",
        QString(), "Spatial Workspace (*.spatial)");
    if (!path.isEmpty()) {
        m_workspaceManager->load(path, m_scene, m_view);
        m_statusLabel->setText("Workspace loaded.");
    }
}

void MainWindow::onCollapseAll()
{
    m_scene->collapseAll();
    m_statusLabel->setText("All nodes collapsed.");
}

void MainWindow::onOpenSettings()
{
    m_stackedWidget->setCurrentWidget(m_settingsView);
    m_statusLabel->setText("Settings opened.");
}

void MainWindow::onCloseSettings()
{
    m_stackedWidget->setCurrentWidget(m_view);
    m_statusLabel->setText("Settings updated.");
}

void MainWindow::onNodeSelected(const QString &path)
{
    m_pathLabel->setText(path);
}

void MainWindow::onSelectionChanged()
{
    QList<QGraphicsItem *> selected = m_scene->selectedItems();

    if (selected.isEmpty()) {
        m_selectionLabel->setText("");
        return;
    }

    int fileCount = 0;
    int folderCount = 0;
    qint64 totalSize = 0;
    QSet<QString> extensions;

    for (QGraphicsItem *item : selected) {
        NodeItem *node = dynamic_cast<NodeItem *>(item);
        if (!node) continue;

        if (node->isDir() || node->isDrive()) {
            folderCount++;
        } else {
            fileCount++;
            totalSize += node->fileSize();
            QString ext = QFileInfo(node->filePath()).suffix().toUpper();
            if (!ext.isEmpty())
                extensions.insert(ext);
        }
    }

    int total = fileCount + folderCount;
    QString info = QString("%1 selected").arg(total);

    // Breakdown
    QStringList parts;
    if (folderCount > 0)
        parts << QString("%1 folder%2").arg(folderCount).arg(folderCount > 1 ? "s" : "");
    if (fileCount > 0)
        parts << QString("%1 file%2").arg(fileCount).arg(fileCount > 1 ? "s" : "");
    if (!parts.isEmpty())
        info += " (" + parts.join(", ") + ")";

    // Total size
    if (totalSize > 0) {
        QString sizeStr;
        if (totalSize < 1024)
            sizeStr = QString("%1 B").arg(totalSize);
        else if (totalSize < 1024 * 1024)
            sizeStr = QString("%1 KB").arg(totalSize / 1024);
        else if (totalSize < 1024LL * 1024 * 1024)
            sizeStr = QString("%1 MB").arg(QString::number(totalSize / (1024.0 * 1024.0), 'f', 1));
        else
            sizeStr = QString("%1 GB").arg(QString::number(totalSize / (1024.0 * 1024.0 * 1024.0), 'f', 2));
        info += "  ·  " + sizeStr;
    }

    // File types
    if (!extensions.isEmpty()) {
        QStringList extList = extensions.values();
        extList.sort();
        if (extList.size() <= 4)
            info += "  ·  " + extList.join(", ");
        else
            info += QString("  ·  %1 types").arg(extList.size());
    }

    m_selectionLabel->setText(info);
}
