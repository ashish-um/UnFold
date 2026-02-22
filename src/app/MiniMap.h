#pragma once

#include <QGraphicsView>
#include <QTimer>

class SpatialScene;
class SpatialView;

class MiniMap : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MiniMap(SpatialScene *scene, SpatialView *mainView, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateViewport();

private:
    void navigateTo(const QPoint &pos);

    SpatialView *m_mainView;
    QTimer m_updateTimer;
};
