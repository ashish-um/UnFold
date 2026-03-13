#pragma once

#include <QGraphicsScene>
#include <QHash>
#include <QFileInfo>

class NodeItem;
class EdgeItem;
class FilesystemWorker;
class FileWatcher;
class LayoutEngine;

class SpatialScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit SpatialScene(FilesystemWorker *worker, FileWatcher *watcher, QObject *parent = nullptr);
    ~SpatialScene();

    void spawnDriveNodes();
    void resetToHome();
    void expandNode(NodeItem *node);
    void collapseNode(NodeItem *node);
    void collapseAll();
    void navigatePath(const QString &path);

    NodeItem *nodeForPath(const QString &path) const;
    QList<NodeItem *> allNodes() const;

    NodeItem *activeExpandedNode() const { return m_activeExpandedNode; }
    bool isInActiveBranch(const NodeItem *node) const;
    LayoutEngine *layoutEngine() const { return m_layoutEngine; }

    struct SavedNodeState {
        QPointF pos;
        bool isExpanded;
    };
    void setSavedNodeState(const QString &path, const QPointF &pos, bool expanded);
    void clearSavedNodeStates();
    void applySavedState(NodeItem *node);

signals:
    void nodeSelected(const QString &path);
    void statusMessage(const QString &msg);
    void layoutChanged();

private slots:
    void onDirectoryLoaded(const QString &path, const QList<QFileInfo> &entries, bool hasMore);
    void onDirectoryError(const QString &path, const QString &error);
    void onFileAdded(const QString &dirPath, const QString &fileName);
    void onFileRemoved(const QString &dirPath, const QString &fileName);

private:
    NodeItem *createNode(const QFileInfo &info, NodeItem *parent);
    EdgeItem *createEdge(NodeItem *parent, NodeItem *child);
    void removeNodeRecursive(NodeItem *node);

    FilesystemWorker *m_fsWorker;
    FileWatcher *m_fileWatcher;
    LayoutEngine *m_layoutEngine;

    QHash<QString, NodeItem *> m_nodeMap;  // path -> node
    NodeItem *m_activeExpandedNode = nullptr;
    QHash<QString, SavedNodeState> m_savedNodeStates;

    QStringList m_navigationQueue;
    void processNavigationQueue();
};
