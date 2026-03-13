#pragma once

#include <QGraphicsView>
#include <QWheelEvent>

class SpatialScene;

class SpatialView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SpatialView(SpatialScene *scene, QWidget *parent = nullptr);

    void resetView();
    qreal currentZoom() const { return m_zoomLevel; }

signals:
    void viewChanged();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void applyZoom(qreal factor, QPointF center);

    qreal m_zoomLevel = 1.0;
    bool m_isPanning = false;
    bool m_didPan = false;
    QPoint m_lastPanPos;

    static constexpr qreal MIN_ZOOM = 0.05;
    static constexpr qreal MAX_ZOOM = 5.0;
    static constexpr qreal ZOOM_STEP = 1.15;
};
