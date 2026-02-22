#include "LayoutEngine.h"
#include "items/NodeItem.h"
#include "items/EdgeItem.h"

#include <QGraphicsScene>
#include <QtMath>

LayoutEngine::LayoutEngine(QObject *parent)
    : QObject(parent)
{
}

void LayoutEngine::layoutChildren(NodeItem *parent, const QList<NodeItem *> &children)
{
    if (children.isEmpty() || !parent)
        return;

    QPointF parentPos = parent->pos();
    int existingChildCount = parent->childNodes().size() - children.size();
    int newChildCount = children.size();
    int totalChildren = existingChildCount + newChildCount;

    QList<QPointF> positions;

    if (totalChildren <= 20) {
        positions = computeRadialLayout(parentPos, newChildCount, existingChildCount);
    } else {
        positions = computeGridLayout(parentPos, newChildCount, existingChildCount);
    }

    // Apply positions with collision resolution
    QGraphicsScene *scene = parent->scene();
    for (int i = 0; i < children.size() && i < positions.size(); ++i) {
        QPointF targetPos = positions[i];

        // Check for collisions with existing scene items
        if (scene) {
            int attempts = 0;
            while (attempts < 10) {
                QList<QGraphicsItem *> colliding = scene->items(
                    QRectF(targetPos.x() - NodeItem::NODE_WIDTH / 2,
                           targetPos.y() - NodeItem::NODE_HEIGHT / 2,
                           NodeItem::NODE_WIDTH, NodeItem::NODE_HEIGHT));

                // Filter out edges and the node itself
                bool hasCollision = false;
                for (QGraphicsItem *item : colliding) {
                    NodeItem *node = dynamic_cast<NodeItem *>(item);
                    if (node && node != children[i] && node != parent) {
                        hasCollision = true;
                        break;
                    }
                }

                if (!hasCollision) break;

                // Shift outward to resolve collision
                QPointF dir = targetPos - parentPos;
                if (dir.manhattanLength() < 1.0)
                    dir = QPointF(H_SPACING, 0);
                dir /= qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
                targetPos += dir * 60.0;
                attempts++;
            }
        }

        children[i]->setPos(targetPos);

        // Update the edge position
        if (children[i]->edgeToParent()) {
            children[i]->edgeToParent()->updatePosition();
        }
    }
}

QList<QPointF> LayoutEngine::computeRadialLayout(QPointF parentPos, int childCount, int existingChildren)
{
    QList<QPointF> positions;

    // Spread children in a semicircle below the parent
    qreal startAngle = M_PI * 0.15;   // ~27 degrees from vertical
    qreal endAngle = M_PI * 0.85;     // ~153 degrees from vertical
    qreal radius = V_SPACING + (childCount * 15);

    // Clamp radius
    radius = qMin(radius, 600.0);
    radius = qMax(radius, V_SPACING);

    for (int i = 0; i < childCount; ++i) {
        qreal t;
        if (childCount == 1)
            t = 0.5;
        else
            t = static_cast<qreal>(existingChildren + i) / static_cast<qreal>(existingChildren + childCount - 1);

        qreal angle = startAngle + t * (endAngle - startAngle);

        qreal x = parentPos.x() + radius * qCos(angle) * (H_SPACING / V_SPACING);
        qreal y = parentPos.y() + radius * qSin(angle);

        positions.append(QPointF(x, y));
    }

    return positions;
}

QList<QPointF> LayoutEngine::computeGridLayout(QPointF parentPos, int childCount, int existingChildren)
{
    QList<QPointF> positions;

    int cols = qMin(static_cast<int>(GRID_COLS_MAX), childCount);
    qreal totalWidth = (cols - 1) * H_SPACING;
    qreal startX = parentPos.x() - totalWidth / 2.0;
    qreal startY = parentPos.y() + V_SPACING;

    // Offset for existing children rows
    int existingRows = (existingChildren > 0) ? ((existingChildren - 1) / cols + 1) : 0;
    startY += existingRows * V_SPACING;

    for (int i = 0; i < childCount; ++i) {
        int row = i / cols;
        int col = i % cols;

        qreal x = startX + col * H_SPACING;
        qreal y = startY + row * V_SPACING;

        positions.append(QPointF(x, y));
    }

    return positions;
}

bool LayoutEngine::hasCollision(const QPointF &pos, const QList<NodeItem *> &exclude) const
{
    Q_UNUSED(pos);
    Q_UNUSED(exclude);
    return false;
}

QPointF LayoutEngine::resolveCollision(const QPointF &pos, const QList<NodeItem *> &exclude) const
{
    Q_UNUSED(exclude);
    return pos;
}
