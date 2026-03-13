#pragma once

#include <QObject>
#include <QString>

class SpatialScene;
class SpatialView;

class WorkspaceManager : public QObject
{
    Q_OBJECT

public:
    explicit WorkspaceManager(QObject *parent = nullptr);

    void save(const QString &filePath, SpatialScene *scene, SpatialView *view);
    void load(const QString &filePath, SpatialScene *scene, SpatialView *view);
    void autoSave(SpatialScene *scene, SpatialView *view);
    bool autoLoad(SpatialScene *scene, SpatialView *view);

signals:
    void saveFinished(const QString &statusMessage);

private:
    QString autoSavePath() const;
};
