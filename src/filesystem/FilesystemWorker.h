#pragma once

#include <QObject>
#include <QFileInfo>
#include <QThread>

class DirectoryReader;

class FilesystemWorker : public QObject
{
    Q_OBJECT

public:
    explicit FilesystemWorker(QObject *parent = nullptr);
    ~FilesystemWorker();

    static constexpr int BATCH_SIZE = 100;

    void fetchDirectory(const QString &path, int offset = 0);

signals:
    void directoryLoaded(const QString &path, const QList<QFileInfo> &entries, bool hasMore);
    void directoryError(const QString &path, const QString &error);
    // Internal signal to forward request to worker thread
    void requestRead(const QString &path, int offset, int batchSize);

private:
    QThread m_workerThread;
    DirectoryReader *m_reader;
};

// Internal worker that runs on a separate thread
class DirectoryReader : public QObject
{
    Q_OBJECT

public slots:
    void readDirectory(const QString &path, int offset, int batchSize);

signals:
    void directoryLoaded(const QString &path, const QList<QFileInfo> &entries, bool hasMore);
    void directoryError(const QString &path, const QString &error);
};
