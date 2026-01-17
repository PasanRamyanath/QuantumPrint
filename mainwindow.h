#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include "FileWatcher.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showAbout();
    void showLicenseInfo();
    void changeLicense();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    FileWatcher *fileWatcher;
};

#endif
