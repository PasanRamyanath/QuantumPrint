#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QWidget>

class FileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcher(QWidget *parent = nullptr);

    void startWatching();

private slots:
    void processPdfFiles();
    void onDirectoryChanged(const QString &path);

private:
    QFileSystemWatcher *watcher;
    QTimer *timer;
    QString tempFolder;
    QWidget *mainWindow;
    QStringList processedFiles;
};

#endif
