#pragma once

#include <QGraphicsItem>

class NodeItem;

class EdgeItem : public QGraphicsItem
{
public:
    EdgeItem(NodeItem *source, NodeItem *dest, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void updatePosition();

    NodeItem *sourceNode() const { return m_source; }
    NodeItem *destNode() const { return m_dest; }

private:
    NodeItem *m_source;
    NodeItem *m_dest;
    QPointF m_sourcePoint;
    QPointF m_destPoint;
};
