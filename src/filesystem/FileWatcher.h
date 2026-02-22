#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QHash>
#include <QStringList>
#include <QTimer>

class FileWatcher : public QObject
{
    Q_OBJECT

public:
    explicit FileWatcher(QObject *parent = nullptr);

    void watchDirectory(const QString &path);
    void unwatchDirectory(const QString &path);

signals:
    void fileAdded(const QString &dirPath, const QString &fileName);
    void fileRemoved(const QString &dirPath, const QString &fileName);

private slots:
    void onDirectoryChanged(const QString &path);

private:
    QFileSystemWatcher m_watcher;
    QHash<QString, QStringList> m_directoryContents;  // path -> list of filenames
    QTimer m_debounceTimer;
    QStringList m_pendingChanges;
};
