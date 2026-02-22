#include "SpatialScene.h"
#include "items/NodeItem.h"
#include "items/EdgeItem.h"
#include "filesystem/FilesystemWorker.h"
#include "filesystem/FileWatcher.h"
#include "layout/LayoutEngine.h"

#include <QDir>
#include <QStorageInfo>

SpatialScene::SpatialScene(FilesystemWorker *worker, FileWatcher *watcher, QObject *parent)
    : QGraphicsScene(parent)
    , m_fsWorker(worker)
    , m_fileWatcher(watcher)
{
    setSceneRect(-50000, -50000, 100000, 100000);
    setBackgroundBrush(QColor(22, 22, 26));

    m_layoutEngine = new LayoutEngine(this);

    // Connect filesystem worker signals
    connect(m_fsWorker, &FilesystemWorker::directoryLoaded,
            this, &SpatialScene::onDirectoryLoaded);
    connect(m_fsWorker, &FilesystemWorker::directoryError,
            this, &SpatialScene::onDirectoryError);

    // Connect file watcher signals
    connect(m_fileWatcher, &FileWatcher::fileAdded,
            this, &SpatialScene::onFileAdded);
    connect(m_fileWatcher, &FileWatcher::fileRemoved,
            this, &SpatialScene::onFileRemoved);
}

SpatialScene::~SpatialScene()
{
}

void SpatialScene::spawnDriveNodes()
{
    const auto volumes = QStorageInfo::mountedVolumes();
    double xOffset = 0;

    for (const QStorageInfo &vol : volumes) {
        if (!vol.isValid() || !vol.isReady())
            continue;

        QString rootPath = QDir::toNativeSeparators(vol.rootPath());
        if (m_nodeMap.contains(rootPath))
            continue;

        QFileInfo info(vol.rootPath());
        NodeItem *driveNode = createNode(info, nullptr);
        driveNode->setPos(xOffset, 0);
        driveNode->setIsDrive(true);

        QString label = vol.displayName();
        if (label.isEmpty())
            label = rootPath;
        else
            label = label + " (" + rootPath + ")";
        driveNode->setLabel(label);

        xOffset += 250;
    }
}

void SpatialScene::resetToHome()
{
    m_activeExpandedNode = nullptr;

    // Remove every node and edge from the scene
    QList<NodeItem *> allNodesCopy = m_nodeMap.values();
    for (NodeItem *node : allNodesCopy) {
        EdgeItem *edge = node->edgeToParent();
        if (edge) {
            removeItem(edge);
            delete edge;
        }
        removeItem(node);
        delete node;
    }
    m_nodeMap.clear();

    // Re-spawn fresh drive nodes at origin
    spawnDriveNodes();
}

void SpatialScene::expandNode(NodeItem *node)
{
    if (!node || !node->isDir() || node->isExpanded())
        return;

    if (node->isSymlink()) {
        emit statusMessage("Symlink: " + node->symlinkTarget() + " (not expanded to prevent loops)");
        return;
    }

    node->setExpanded(true);

    // Mark as the active (most recently expanded) node and refresh all
    m_activeExpandedNode = node;
    update();  // trigger repaint on all items

    emit statusMessage("Loading: " + node->filePath() + "...");

    // Start watching this directory
    m_fileWatcher->watchDirectory(node->filePath());

    // Fetch contents asynchronously
    m_fsWorker->fetchDirectory(node->filePath());
}

void SpatialScene::collapseNode(NodeItem *node)
{
    if (!node || !node->isExpanded())
        return;

    node->setExpanded(false);
    m_fileWatcher->unwatchDirectory(node->filePath());

    // If collapsing the active node, clear active focus
    if (m_activeExpandedNode == node) {
        m_activeExpandedNode = node->parentNode();
        update();
    }

    // Remove all children recursively
    QList<NodeItem *> children = node->childNodes();
    for (NodeItem *child : children) {
        removeNodeRecursive(child);
    }
    node->clearChildren();

    emit statusMessage("Collapsed: " + node->filePath());
}

void SpatialScene::collapseAll()
{
    m_activeExpandedNode = nullptr;

    QList<NodeItem *> roots;
    for (NodeItem *node : m_nodeMap) {
        if (!node->parentNode()) {
            roots.append(node);
        }
    }
    for (NodeItem *root : roots) {
        collapseNode(root);
    }
    update();
}

NodeItem *SpatialScene::nodeForPath(const QString &path) const
{
    return m_nodeMap.value(QDir::toNativeSeparators(path), nullptr);
}

QList<NodeItem *> SpatialScene::allNodes() const
{
    return m_nodeMap.values();
}

void SpatialScene::onDirectoryLoaded(const QString &path, const QList<QFileInfo> &entries, bool hasMore)
{
    NodeItem *parentNode = nodeForPath(path);
    if (!parentNode) return;

    QList<NodeItem *> newChildren;

    for (const QFileInfo &info : entries) {
        QString childPath = QDir::toNativeSeparators(info.absoluteFilePath());
        if (m_nodeMap.contains(childPath))
            continue;

        NodeItem *childNode = createNode(info, parentNode);
        EdgeItem *edge = createEdge(parentNode, childNode);
        childNode->setEdgeToParent(edge);
        parentNode->addChild(childNode);
        newChildren.append(childNode);
    }

    if (hasMore) {
        // Create a "Load more..." placeholder node
        QFileInfo loadMoreInfo;
        NodeItem *loadMore = new NodeItem(loadMoreInfo);
        loadMore->setIsLoadMore(true);
        loadMore->setParentPath(path);
        loadMore->setLabel("Load more...");
        addItem(loadMore);

        EdgeItem *edge = createEdge(parentNode, loadMore);
        loadMore->setEdgeToParent(edge);
        parentNode->addChild(loadMore);
        newChildren.append(loadMore);
    }

    // Use layout engine to position children
    m_layoutEngine->layoutChildren(parentNode, newChildren);

    int count = entries.size();
    QString msg = QString("Loaded %1 items from %2").arg(count).arg(path);
    if (hasMore) msg += " (more available)";
    emit statusMessage(msg);
}

void SpatialScene::onDirectoryError(const QString &path, const QString &error)
{
    NodeItem *node = nodeForPath(path);
    if (node) {
        node->setPermissionDenied(true);
        node->setExpanded(false);
    }
    emit statusMessage("Error: " + error);
}

void SpatialScene::onFileAdded(const QString &dirPath, const QString &fileName)
{
    NodeItem *parentNode = nodeForPath(dirPath);
    if (!parentNode || !parentNode->isExpanded())
        return;

    QString fullPath = QDir::toNativeSeparators(dirPath + "/" + fileName);
    if (m_nodeMap.contains(fullPath))
        return;

    QFileInfo info(fullPath);
    if (!info.exists()) return;

    NodeItem *childNode = createNode(info, parentNode);
    EdgeItem *edge = createEdge(parentNode, childNode);
    childNode->setEdgeToParent(edge);
    parentNode->addChild(childNode);

    m_layoutEngine->layoutChildren(parentNode, {childNode});
    emit statusMessage("File added: " + fileName);
}

void SpatialScene::onFileRemoved(const QString &dirPath, const QString &fileName)
{
    QString fullPath = QDir::toNativeSeparators(dirPath + "/" + fileName);
    NodeItem *node = nodeForPath(fullPath);
    if (!node) return;

    NodeItem *parent = node->parentNode();
    if (parent) {
        parent->removeChild(node);
    }
    removeNodeRecursive(node);
    emit statusMessage("File removed: " + fileName);
}

NodeItem *SpatialScene::createNode(const QFileInfo &info, NodeItem *parent)
{
    NodeItem *node = new NodeItem(info);
    node->setScene(this);
    addItem(node);

    QString path = QDir::toNativeSeparators(info.absoluteFilePath());
    m_nodeMap.insert(path, node);

    return node;
}

EdgeItem *SpatialScene::createEdge(NodeItem *parent, NodeItem *child)
{
    EdgeItem *edge = new EdgeItem(parent, child);
    addItem(edge);
    return edge;
}

void SpatialScene::removeNodeRecursive(NodeItem *node)
{
    if (!node) return;

    // First remove all children
    QList<NodeItem *> children = node->childNodes();
    for (NodeItem *child : children) {
        removeNodeRecursive(child);
    }

    // Unwatch if it was expanded
    if (node->isExpanded()) {
        m_fileWatcher->unwatchDirectory(node->filePath());
    }

    // Remove from map
    m_nodeMap.remove(QDir::toNativeSeparators(node->filePath()));

    // Remove edge to parent
    EdgeItem *edge = node->edgeToParent();
    if (edge) {
        removeItem(edge);
        delete edge;
    }

    // Remove node from scene
    removeItem(node);
    delete node;
}

bool SpatialScene::isInActiveBranch(const NodeItem *node) const
{
    if (!m_activeExpandedNode)
        return true;  // no active node means everything is bright

    // The active node itself and its direct children are in the active branch
    if (node == m_activeExpandedNode)
        return true;
    if (node->parentNode() == m_activeExpandedNode)
        return true;

    // Walk up from the active node — ancestors of the active node are also considered active
    const NodeItem *ancestor = m_activeExpandedNode;
    while (ancestor) {
        if (ancestor == node)
            return true;
        ancestor = ancestor->parentNode();
    }

    return false;
}
