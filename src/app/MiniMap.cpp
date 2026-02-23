#include "MiniMap.h"
#include "canvas/SpatialView.h"
#include "canvas/SpatialScene.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>

MiniMap::MiniMap(SpatialScene *scene, SpatialView *mainView, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_mainView(mainView)
{
    setFixedSize(220, 160);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(false);
    setRenderHints(QPainter::Antialiasing);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);

    setStyleSheet(
        "QGraphicsView { background: rgba(20, 20, 20, 200); "
        "border: 1px solid #4a4a4a; border-radius: 6px; }"
    );

    // Watch parent for resize to stay anchored at bottom-right
    if (parent) {
        parent->installEventFilter(this);
        repositionInParent();
    }

    // Update periodically
    m_updateTimer.setInterval(200);
    connect(&m_updateTimer, &QTimer::timeout, this, &MiniMap::updateViewport);
    m_updateTimer.start();
}

void MiniMap::updateViewport()
{
    // Fit all items into the minimap view
    QRectF itemsBounds = scene()->itemsBoundingRect();
    if (!itemsBounds.isEmpty()) {
        itemsBounds.adjust(-200, -200, 200, 200);
        fitInView(itemsBounds, Qt::KeepAspectRatio);
    }
    viewport()->update();
}

void MiniMap::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        navigateTo(event->pos());
    }
}

void MiniMap::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        navigateTo(event->pos());
    }
}

void MiniMap::navigateTo(const QPoint &pos)
{
    QPointF scenePos = mapToScene(pos);
    m_mainView->centerOn(scenePos);
}

void MiniMap::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

    // Draw the main viewport rectangle
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF viewportRect = m_mainView->mapToScene(m_mainView->viewport()->rect()).boundingRect();
    QRectF mapped = QRectF(mapFromScene(viewportRect.topLeft()), mapFromScene(viewportRect.bottomRight()));

    painter.setPen(QPen(QColor(100, 160, 230, 180), 2));
    painter.setBrush(QColor(100, 160, 230, 30));
    painter.drawRect(mapped);
}

void MiniMap::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateViewport();
}

bool MiniMap::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize) {
        repositionInParent();
    }
    return QGraphicsView::eventFilter(watched, event);
}

void MiniMap::repositionInParent()
{
    if (parentWidget()) {
        int x = parentWidget()->width() - width() - 12;
        int y = parentWidget()->height() - height() - 12;
        move(x, y);
    }
}
