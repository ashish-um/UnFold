#include "EdgeItem.h"
#include "NodeItem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

EdgeItem::EdgeItem(NodeItem *source, NodeItem *dest, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_source(source)
    , m_dest(dest)
{
    setZValue(1);  // Below nodes
    setCacheMode(DeviceCoordinateCache);
    updatePosition();
}

void EdgeItem::updatePosition()
{
    prepareGeometryChange();
    m_sourcePoint = m_source->pos();
    m_destPoint = m_dest->pos();
}

QRectF EdgeItem::boundingRect() const
{
    qreal extra = 4.0;
    return QRectF(m_sourcePoint, m_destPoint).normalized().adjusted(-extra, -extra, extra, extra);
}

void EdgeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());

    // Skip rendering at very low LOD
    if (lod < 0.1)
        return;

    painter->setRenderHint(QPainter::Antialiasing);

    // Calculate control points for Bezier curve
    QPointF src = m_sourcePoint;
    QPointF dst = m_destPoint;
    qreal dy = dst.y() - src.y();
    qreal dx = dst.x() - src.x();

    // The curve drops down from the parent and curves into the child
    QPointF c1, c2;
    if (qAbs(dy) > qAbs(dx)) {
        // Mostly vertical connection
        c1 = QPointF(src.x(), src.y() + dy * 0.5);
        c2 = QPointF(dst.x(), dst.y() - dy * 0.5);
    } else {
        // Mostly horizontal connection
        c1 = QPointF(src.x() + dx * 0.5, src.y());
        c2 = QPointF(dst.x() - dx * 0.5, dst.y());
    }

    QPainterPath path;
    path.moveTo(src);
    path.cubicTo(c1, c2, dst);

    // Draw the curve
    QColor lineColor(100, 120, 150, 120);
    qreal lineWidth = 1.5;

    if (lod < 0.3) {
        lineColor.setAlpha(60);
        lineWidth = 1.0;
    }

    painter->setPen(QPen(lineColor, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    // Draw arrowhead at destination (only at decent zoom)
    if (lod > 0.3) {
        qreal arrowSize = 6.0;
        QLineF line(c2, dst);
        double angle = std::atan2(-line.dy(), line.dx());

        QPointF arrowP1 = dst + QPointF(std::sin(angle - M_PI / 3) * arrowSize,
                                         std::cos(angle - M_PI / 3) * arrowSize);
        QPointF arrowP2 = dst + QPointF(std::sin(angle - M_PI + M_PI / 3) * arrowSize,
                                         std::cos(angle - M_PI + M_PI / 3) * arrowSize);

        QPolygonF arrowHead;
        arrowHead << dst << arrowP1 << arrowP2;
        painter->setPen(Qt::NoPen);
        painter->setBrush(lineColor);
        painter->drawPolygon(arrowHead);
    }
}
