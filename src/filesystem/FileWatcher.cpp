#include "FileWatcher.h"

#include <QDir>
#include <QSet>

FileWatcher::FileWatcher(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &FileWatcher::onDirectoryChanged);

    // Debounce rapid filesystem changes
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(300);
    connect(&m_debounceTimer, &QTimer::timeout, this, [this]() {
        for (const QString &path : m_pendingChanges) {
            onDirectoryChanged(path);
        }
        m_pendingChanges.clear();
    });
}

void FileWatcher::watchDirectory(const QString &path)
{
    if (m_directoryContents.contains(path))
        return;

    QDir dir(path);
    if (!dir.exists() || !dir.isReadable())
        return;

    m_watcher.addPath(path);

    // Snapshot current contents
    QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    m_directoryContents[path] = entries;
}

void FileWatcher::unwatchDirectory(const QString &path)
{
    if (!m_directoryContents.contains(path))
        return;

    m_watcher.removePath(path);
    m_directoryContents.remove(path);
}

void FileWatcher::onDirectoryChanged(const QString &path)
{
    if (!m_directoryContents.contains(path))
        return;

    QDir dir(path);
    if (!dir.exists()) return;

    QStringList newEntries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    QStringList oldEntries = m_directoryContents[path];

    QSet<QString> newSet(newEntries.begin(), newEntries.end());
    QSet<QString> oldSet(oldEntries.begin(), oldEntries.end());

    // Find added files
    QSet<QString> added = newSet - oldSet;
    for (const QString &name : added) {
        emit fileAdded(path, name);
    }

    // Find removed files
    QSet<QString> removed = oldSet - newSet;
    for (const QString &name : removed) {
        emit fileRemoved(path, name);
    }

    // Update snapshot
    m_directoryContents[path] = newEntries;
}
