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

    // Determine direction: if this node is above its parent, expand upward
    bool flipVertical = false;
    if (parent->parentNode()) {
        if (parent->pos().y() < parent->parentNode()->pos().y()) {
            flipVertical = true;
        }
    }

    QList<QPointF> positions;

    if (totalChildren <= 20) {
        positions = computeRadialLayout(parentPos, newChildCount, existingChildCount, flipVertical);
    } else {
        positions = computeGridLayout(parentPos, newChildCount, existingChildCount, flipVertical);
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

QList<QPointF> LayoutEngine::computeRadialLayout(QPointF parentPos, int childCount, int existingChildren, bool flipVertical)
{
    QList<QPointF> positions;
    if (childCount == 0) return positions;

    qreal startAngle = M_PI * 0.15;   // ~27 degrees
    qreal endAngle = M_PI * 0.85;     // ~153 degrees
    qreal arcSpan = endAngle - startAngle;
    qreal minNodeSpacing = NodeItem::NODE_WIDTH + 20.0;

    qreal baseRadius = V_SPACING + 20.0;
    qreal ringGap = NodeItem::NODE_HEIGHT + 30.0;

    // First pass: figure out how many nodes go on each ring
    QList<int> ringCounts;
    int remaining = childCount;
    int ring = 0;
    while (remaining > 0) {
        qreal radius = baseRadius + ring * ringGap;
        int capacity = qMax(1, static_cast<int>(radius * arcSpan / minNodeSpacing) + 1);
        int countOnRing = qMin(capacity, remaining);
        ringCounts.append(countOnRing);
        remaining -= countOnRing;
        ring++;
    }

    // Second pass: place nodes, staggering odd rings by half a slot
    for (int r = 0; r < ringCounts.size(); ++r) {
        qreal radius = baseRadius + r * ringGap;
        int countOnRing = ringCounts[r];

        // Tighten the angular span for rings with fewer nodes
        // so they cluster near the center instead of fanning out
        qreal neededArc = (countOnRing - 1) * minNodeSpacing / radius;
        qreal ringArc = qMin(arcSpan, qMax(neededArc, arcSpan * 0.3));
        qreal ringStart = (M_PI - ringArc) / 2.0;  // center the arc

        for (int i = 0; i < countOnRing; ++i) {
            qreal t;
            if (countOnRing == 1)
                t = 0.5;
            else
                t = static_cast<qreal>(i) / static_cast<qreal>(countOnRing - 1);

            qreal angle = ringStart + t * ringArc;

            // Stagger odd rings by half a slot to fill gaps
            if (r % 2 == 1 && countOnRing > 1) {
                qreal halfSlot = ringArc / (countOnRing - 1) * 0.5;
                angle += halfSlot;
                // Pull the ends inward so they don't exceed the arc
                if (i == 0) angle -= halfSlot * 0.3;
                if (i == countOnRing - 1) angle -= halfSlot * 0.7;
            }

            qreal x = parentPos.x() + radius * qCos(angle) * (H_SPACING / V_SPACING);
            qreal yOffset = radius * qSin(angle);
            qreal y = parentPos.y() + (flipVertical ? -yOffset : yOffset);

            positions.append(QPointF(x, y));
        }
    }

    return positions;
}

QList<QPointF> LayoutEngine::computeGridLayout(QPointF parentPos, int childCount, int existingChildren, bool flipVertical)
{
    QList<QPointF> positions;

    int cols = qMin(static_cast<int>(GRID_COLS_MAX), childCount);
    qreal totalWidth = (cols - 1) * H_SPACING;
    qreal startX = parentPos.x() - totalWidth / 2.0;
    qreal yDir = flipVertical ? -1.0 : 1.0;
    qreal startY = parentPos.y() + V_SPACING * yDir;

    // Offset for existing children rows
    int existingRows = (existingChildren > 0) ? ((existingChildren - 1) / cols + 1) : 0;
    startY += existingRows * V_SPACING * yDir;

    for (int i = 0; i < childCount; ++i) {
        int row = i / cols;
        int col = i % cols;

        qreal x = startX + col * H_SPACING;
        qreal y = startY + row * V_SPACING * yDir;

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

void LayoutEngine::relayoutAroundParent(NodeItem *parent, const QPointF &delta)
{
    if (!parent || !parent->isExpanded())
        return;

    QList<NodeItem *> allChildren = parent->childNodes();
    if (allChildren.isEmpty())
        return;

    // Separate expanded vs collapsed children
    QList<NodeItem *> collapsedChildren;
    QList<NodeItem *> expandedChildren;
    for (NodeItem *child : allChildren) {
        if (child->isExpanded()) {
            expandedChildren.append(child);
        } else {
            collapsedChildren.append(child);
        }
    }

    // Move expanded children rigidly — they keep their subtree intact
    for (NodeItem *child : expandedChildren) {
        child->m_movingChildren = true;
        child->moveBy(delta.x(), delta.y());
        child->moveChildrenBy(delta);
        child->m_movingChildren = false;
    }

    // Re-layout only collapsed children around the parent's new position
    if (collapsedChildren.isEmpty())
        return;

    QPointF parentPos = parent->pos();
    int childCount = collapsedChildren.size();

    // Determine direction relative to grandparent
    bool flipVertical = false;
    if (parent->parentNode()) {
        if (parent->pos().y() < parent->parentNode()->pos().y()) {
            flipVertical = true;
        }
    }

    QList<QPointF> positions;
    if (childCount <= 20) {
        positions = computeRadialLayout(parentPos, childCount, 0, flipVertical);
    } else {
        positions = computeGridLayout(parentPos, childCount, 0, flipVertical);
    }

    // Collect all descendants to exclude from collision checks
    QSet<QGraphicsItem *> familySet;
    familySet.insert(parent);
    collectDescendants(parent, familySet);

    QGraphicsScene *scene = parent->scene();

    for (int i = 0; i < collapsedChildren.size() && i < positions.size(); ++i) {
        QPointF targetPos = positions[i];

        // Resolve collisions against non-family nodes
        if (scene) {
            int attempts = 0;
            while (attempts < 10) {
                QList<QGraphicsItem *> colliding = scene->items(
                    QRectF(targetPos.x() - NodeItem::NODE_WIDTH / 2,
                           targetPos.y() - NodeItem::NODE_HEIGHT / 2,
                           NodeItem::NODE_WIDTH, NodeItem::NODE_HEIGHT));

                bool hasCollision = false;
                for (QGraphicsItem *item : colliding) {
                    if (!familySet.contains(item) && dynamic_cast<NodeItem *>(item)) {
                        hasCollision = true;
                        break;
                    }
                }

                if (!hasCollision) break;

                QPointF dir = targetPos - parentPos;
                if (dir.manhattanLength() < 1.0)
                    dir = QPointF(H_SPACING, 0);
                qreal len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
                dir /= len;
                targetPos += dir * 60.0;
                attempts++;
            }
        }

        collapsedChildren[i]->m_movingChildren = true;
        collapsedChildren[i]->setPos(targetPos);
        collapsedChildren[i]->m_movingChildren = false;
    }
}

void LayoutEngine::collectDescendants(NodeItem *node, QSet<QGraphicsItem *> &out) const
{
    for (NodeItem *child : node->childNodes()) {
        out.insert(child);
        collectDescendants(child, out);
    }
}
