#pragma once

#include <QGraphicsItem>
#include <QFileInfo>
#include <QList>
#include <QPen>

class EdgeItem;
class SpatialScene;
class LayoutEngine;

class NodeItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    friend class LayoutEngine;

public:
    explicit NodeItem(const QFileInfo &fileInfo, QGraphicsItem *parent = nullptr);
    ~NodeItem() override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    // Accessors
    QString filePath() const { return m_filePath; }
    QString fileName() const { return m_fileName; }
    bool isDir() const { return m_isDir; }
    bool isExpanded() const { return m_isExpanded; }
    bool isSymlink() const { return m_isSymlink; }
    bool isDrive() const { return m_isDrive; }
    bool isLoadMore() const { return m_isLoadMore; }
    bool isPermissionDenied() const { return m_permissionDenied; }
    QString symlinkTarget() const { return m_symlinkTarget; }
    qint64 fileSize() const { return m_fileSize; }

    // Setters
    void setExpanded(bool expanded);
    void setIsDrive(bool drive) { m_isDrive = drive; }
    void setIsLoadMore(bool loadMore) { m_isLoadMore = loadMore; }
    void setPermissionDenied(bool denied);
    void setLabel(const QString &label) { m_label = label; update(); }
    void setParentPath(const QString &path) { m_parentPath = path; }

    // Scene reference
    void setScene(SpatialScene *scene) { m_spatialScene = scene; }
    SpatialScene *spatialScene() const { return m_spatialScene; }

    // Tree structure
    NodeItem *parentNode() const { return m_parentNode; }
    void setParentNode(NodeItem *parent) { m_parentNode = parent; }
    QList<NodeItem *> childNodes() const { return m_children; }
    void addChild(NodeItem *child);
    void removeChild(NodeItem *child);
    void clearChildren() { m_children.clear(); }

    // Edge
    EdgeItem *edgeToParent() const { return m_edgeToParent; }
    void setEdgeToParent(EdgeItem *edge) { m_edgeToParent = edge; }

    // For "Load more" nodes
    QString parentPath() const { return m_parentPath; }

    // For layout engine access during drag re-layout
    void moveChildrenBy(const QPointF &delta);

    static constexpr qreal NODE_WIDTH = 150.0;
    static constexpr qreal NODE_HEIGHT = 56.0;

signals:
    void clicked(NodeItem *node);
    void doubleClicked(NodeItem *node);
    void nodeMoved();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    void paintNode(QPainter *painter, qreal lod);
    void paintIcon(QPainter *painter, const QRectF &iconRect);
    QColor nodeColor() const;
    QString fileTypeLabel() const;

    QFileInfo m_fileInfo;
    QString m_filePath;
    QString m_fileName;
    QString m_label;
    QString m_parentPath;
    QString m_symlinkTarget;
    qint64 m_fileSize = 0;

    bool m_isDir = false;
    bool m_isExpanded = false;
    bool m_isSymlink = false;
    bool m_isDrive = false;
    bool m_isLoadMore = false;
    bool m_permissionDenied = false;
    bool m_isHovered = false;
    bool m_movingChildren = false;
    bool m_isDragging = false;
    QPointF m_lastPos;

    NodeItem *m_parentNode = nullptr;
    QList<NodeItem *> m_children;
    EdgeItem *m_edgeToParent = nullptr;
    SpatialScene *m_spatialScene = nullptr;
};
