#include "SpatialView.h"
#include "SpatialScene.h"

#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QtMath>

SpatialView::SpatialView(SpatialScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);
    setCacheMode(QGraphicsView::CacheBackground);

    setBackgroundBrush(QColor(22, 22, 26));

    // Set large scene rect for "infinite" canvas feel
    setSceneRect(-50000, -50000, 100000, 100000);
}

void SpatialView::resetView()
{
    resetTransform();
    m_zoomLevel = 1.0;
    centerOn(0, 0);
}

void SpatialView::wheelEvent(QWheelEvent *event)
{
    QPointF scenePosBefore = mapToScene(event->position().toPoint());

    qreal factor = (event->angleDelta().y() > 0) ? ZOOM_STEP : (1.0 / ZOOM_STEP);
    qreal newZoom = m_zoomLevel * factor;

    if (newZoom < MIN_ZOOM || newZoom > MAX_ZOOM)
        return;

    m_zoomLevel = newZoom;
    scale(factor, factor);

    // Keep the point under cursor stationary
    QPointF scenePosAfter = mapToScene(event->position().toPoint());
    QPointF delta = scenePosAfter - scenePosBefore;
    translate(delta.x(), delta.y());

    event->accept();
}

void SpatialView::mousePressEvent(QMouseEvent *event)
{
    // Right-click or middle-click: pan the canvas
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_didPan = false;
        m_lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    // Left-click on empty canvas: start rubber band selection
    if (event->button() == Qt::LeftButton) {
        QGraphicsItem *item = itemAt(event->pos());
        if (!item) {
            setDragMode(QGraphicsView::RubberBandDrag);
            QGraphicsView::mousePressEvent(event);
            return;
        }
    }

    // Left-click on an item: default behavior (select/move)
    QGraphicsView::mousePressEvent(event);
}

void SpatialView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPointF delta = mapToScene(event->pos()) - mapToScene(m_lastPanPos);
        m_lastPanPos = event->pos();
        m_didPan = true;

        setTransformationAnchor(QGraphicsView::NoAnchor);
        translate(delta.x(), delta.y());

        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void SpatialView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    // Reset drag mode after rubber band selection completes
    if (dragMode() == QGraphicsView::RubberBandDrag) {
        QGraphicsView::mouseReleaseEvent(event);
        setDragMode(QGraphicsView::NoDrag);
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void SpatialView::contextMenuEvent(QContextMenuEvent *event)
{
    // Suppress context menu if we just panned with right-click
    if (m_didPan) {
        m_didPan = false;
        event->accept();
        return;
    }
    QGraphicsView::contextMenuEvent(event);
}

void SpatialView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void SpatialView::applyZoom(qreal factor, QPointF center)
{
    qreal newZoom = m_zoomLevel * factor;
    if (newZoom < MIN_ZOOM || newZoom > MAX_ZOOM)
        return;

    m_zoomLevel = newZoom;

    QPointF scenePosBefore = mapToScene(center.toPoint());
    scale(factor, factor);
    QPointF scenePosAfter = mapToScene(center.toPoint());
    QPointF delta = scenePosAfter - scenePosBefore;
    translate(delta.x(), delta.y());
}
