#pragma once

#include <QObject>
#include <QList>
#include <QPointF>
#include <QSet>

class NodeItem;
class QGraphicsItem;

class LayoutEngine : public QObject
{
    Q_OBJECT

public:
    explicit LayoutEngine(QObject *parent = nullptr);

    void layoutChildren(NodeItem *parent, const QList<NodeItem *> &children);
    void relayoutAroundParent(NodeItem *parent, const QPointF &delta);

    static constexpr qreal H_SPACING = 180.0;
    static constexpr qreal V_SPACING = 130.0;
    static constexpr qreal GRID_COLS_MAX = 8;

private:
    QList<QPointF> computeRadialLayout(QPointF parentPos, int childCount, int existingChildren);
    QList<QPointF> computeGridLayout(QPointF parentPos, int childCount, int existingChildren);
    bool hasCollision(const QPointF &pos, const QList<NodeItem *> &exclude) const;
    QPointF resolveCollision(const QPointF &pos, const QList<NodeItem *> &exclude) const;
    void collectDescendants(NodeItem *node, QSet<QGraphicsItem *> &out) const;
};
