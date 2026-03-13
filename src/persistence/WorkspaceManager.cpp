#include "WorkspaceManager.h"
#include "canvas/SpatialScene.h"
#include "canvas/SpatialView.h"
#include "items/NodeItem.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTransform>
#include <QtConcurrent>

WorkspaceManager::WorkspaceManager(QObject *parent)
    : QObject(parent)
{
}

void WorkspaceManager::save(const QString &filePath, SpatialScene *scene, SpatialView *view)
{
    QJsonObject root;
    root["version"] = 1;

    // Save view transform
    QTransform t = view->transform();
    QJsonObject viewObj;
    viewObj["m11"] = t.m11();
    viewObj["m12"] = t.m12();
    viewObj["m21"] = t.m21();
    viewObj["m22"] = t.m22();
    viewObj["dx"] = t.dx();
    viewObj["dy"] = t.dy();
    root["viewTransform"] = viewObj;

    // Save all nodes
    QJsonArray nodesArray;
    QList<NodeItem *> allNodes = scene->allNodes();
    for (NodeItem *node : allNodes) {
        if (node->isLoadMore()) continue;

        QJsonObject nodeObj;
        nodeObj["path"] = node->filePath();
        nodeObj["x"] = node->pos().x();
        nodeObj["y"] = node->pos().y();
        nodeObj["expanded"] = node->isExpanded();
        nodeObj["isDrive"] = node->isDrive();
        nodesArray.append(nodeObj);
    }
    root["nodes"] = nodesArray;

    // Serialize JSON synchronously on main thread
    QJsonDocument doc(root);
    QByteArray jsonBytes = doc.toJson();

    // Write to file asynchronously to avoid blocking UI
    QtConcurrent::run([this, filePath, jsonBytes]() {
        QFile file(filePath);
        int lineCount = 0;
        if (file.open(QIODevice::WriteOnly)) {
            file.write(jsonBytes);
            file.close();
            lineCount = jsonBytes.count('\n') + 1;
            
            // Emit success via cross-thread queued event back to main thread
            QString statusMessage = QString("Saved to %1 (%2 lines)").arg(filePath).arg(lineCount);
            emit saveFinished(statusMessage);
        } else {
            emit saveFinished(QString("Failed to save workspace to %1").arg(filePath));
        }
    });
}

void WorkspaceManager::load(const QString &filePath, SpatialScene *scene, SpatialView *view)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();

    // Restore view transform
    if (root.contains("viewTransform")) {
        QJsonObject viewObj = root["viewTransform"].toObject();
        QTransform t(
            viewObj["m11"].toDouble(1), viewObj["m12"].toDouble(0),
            viewObj["m21"].toDouble(0), viewObj["m22"].toDouble(1),
            viewObj["dx"].toDouble(0), viewObj["dy"].toDouble(0)
        );
        view->setTransform(t);
    }

    scene->clearSavedNodeStates();

    // Cache node states
    if (root.contains("nodes")) {
        QJsonArray nodesArray = root["nodes"].toArray();

        // Sort by path length so parents are processed before children
        QList<QJsonObject> nodeObjects;
        for (const QJsonValue &val : nodesArray) {
            nodeObjects.append(val.toObject());
        }
        std::sort(nodeObjects.begin(), nodeObjects.end(),
            [](const QJsonObject &a, const QJsonObject &b) {
                return a["path"].toString().length() < b["path"].toString().length();
            });

        for (const QJsonObject &nodeObj : nodeObjects) {
            QString path = nodeObj["path"].toString();
            double x = nodeObj["x"].toDouble();
            double y = nodeObj["y"].toDouble();
            bool expanded = nodeObj["expanded"].toBool();
            scene->setSavedNodeState(path, QPointF(x, y), expanded);
        }
    }
    
    // Clear the current scene and spawn drives; they will automatically pick up saved states.
    scene->resetToHome();
}

void WorkspaceManager::autoSave(SpatialScene *scene, SpatialView *view)
{
    QString path = autoSavePath();
    if (!path.isEmpty()) {
        save(path, scene, view);
    } else {
        emit saveFinished("Failed to determine save path.");
    }
}

bool WorkspaceManager::autoLoad(SpatialScene *scene, SpatialView *view)
{
    QString path = autoSavePath();
    if (!path.isEmpty() && QFile::exists(path)) {
        load(path, scene, view);
        return true;
    }
    return false;
}

QString WorkspaceManager::autoSavePath() const
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appData);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appData + "/autosave.spatial";
}
