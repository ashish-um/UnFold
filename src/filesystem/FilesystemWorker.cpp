#include "FilesystemWorker.h"

#include <QDir>
#include <QFileInfo>

FilesystemWorker::FilesystemWorker(QObject *parent)
    : QObject(parent)
{
    m_reader = new DirectoryReader();
    m_reader->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, m_reader, &QObject::deleteLater);

    // Forward the fetch request to the reader on the worker thread
    connect(this, &FilesystemWorker::requestRead,
            m_reader, &DirectoryReader::readDirectory, Qt::QueuedConnection);

    // Forward results back to the main thread
    connect(m_reader, &DirectoryReader::directoryLoaded,
            this, &FilesystemWorker::directoryLoaded, Qt::QueuedConnection);
    connect(m_reader, &DirectoryReader::directoryError,
            this, &FilesystemWorker::directoryError, Qt::QueuedConnection);

    m_workerThread.start();
}

FilesystemWorker::~FilesystemWorker()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void FilesystemWorker::fetchDirectory(const QString &path, int offset, bool showHidden)
{
    emit requestRead(path, offset, BATCH_SIZE, showHidden);
}

// --- DirectoryReader ---

void DirectoryReader::readDirectory(const QString &path, int offset, int batchSize, bool showHidden)
{
    QDir dir(path);
    if (!dir.exists()) {
        emit directoryError(path, "Directory does not exist: " + path);
        return;
    }

    if (!dir.isReadable()) {
        emit directoryError(path, "Permission denied: " + path);
        return;
    }

    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    if (showHidden) {
        filters |= QDir::Hidden;
    }
    QDir::SortFlags sortFlags = QDir::DirsFirst | QDir::Name | QDir::IgnoreCase;

    QFileInfoList allEntries = dir.entryInfoList(filters, sortFlags);

    int total = allEntries.size();
    int start = offset;
    int end = qMin(start + batchSize, total);

    if (start >= total) {
        emit directoryLoaded(path, {}, false);
        return;
    }

    QList<QFileInfo> batch;
    batch.reserve(end - start);
    for (int i = start; i < end; ++i) {
        batch.append(allEntries[i]);
    }

    bool hasMore = (end < total);
    emit directoryLoaded(path, batch, hasMore);
}
