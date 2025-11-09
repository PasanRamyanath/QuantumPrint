#include "FileWatcher.h"
#include "PrinterSelectionDialog.h"
#ifdef HAS_QTPDF
#include "PdfPrinter.h"
#endif
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include "Config.h"

FileWatcher::FileWatcher(QWidget *parent)
    : QObject(parent), mainWindow(parent)
{
    QCoreApplication::setOrganizationName("IMPJR");
    QCoreApplication::setApplicationName("IMPJR_Printer");

    // Default CutePDF output folder in Documents
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    tempFolder = documentsPath + "/QuantumPrint";

    QDir dir(tempFolder);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    qDebug() << "Watching folder:" << tempFolder;

    watcher = new QFileSystemWatcher(this);
    watcher->addPath(tempFolder);

    connect(watcher, &QFileSystemWatcher::directoryChanged,
            this, &FileWatcher::onDirectoryChanged);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FileWatcher::processPdfFiles);
    timer->start(3000); // check every 3 seconds
}

void FileWatcher::startWatching()
{
    qDebug() << "File watching started";
}

void FileWatcher::onDirectoryChanged(const QString &path)
{
    qDebug() << "Directory changed:" << path;
    QTimer::singleShot(1000, this, &FileWatcher::processPdfFiles);
}

void FileWatcher::processPdfFiles()
{
#ifdef HAS_QTPDF
    QDir dir(tempFolder);
    QStringList pdfFiles = dir.entryList(QStringList() << "*.pdf", QDir::Files, QDir::Time);

    for (const QString &file : pdfFiles) {
        QString fullPath = tempFolder + "/" + file;

        if (processedFiles.contains(fullPath))
            continue;

        // Check if file is still being written
        QFile pdfFile(fullPath);
        qint64 size1 = pdfFile.size();
        QThread::msleep(500);
        qint64 size2 = pdfFile.size();

        if (size1 != size2 || size1 == 0) {
            qDebug() << "File still being written:" << file;
            continue;
        }

        qDebug() << "Processing PDF:" << file;
        processedFiles.append(fullPath);

        // Get available printers
        QStringList availablePrinters = Config::getAvailablePrinters();
        if (availablePrinters.isEmpty()) {
            QMessageBox::critical(mainWindow, "No Printer Available",
                                  "No printers found!\nPlease ensure a printer is installed.");
            processedFiles.removeOne(fullPath);
            continue;
        }

        // Show printer selection dialog
        PrinterSelectionDialog dialog(file, availablePrinters, mainWindow);
        QString savedPrinter = Config::getPrinterName();
        int printerIndex = availablePrinters.indexOf(savedPrinter);
        if (printerIndex >= 0)
            dialog.findChild<QComboBox*>()->setCurrentIndex(printerIndex);

        int result = dialog.exec();

        if (result == QDialog::Rejected) {
            qDebug() << "User cancelled print job";
            if (QFile::exists(fullPath)) {
                if (QFile::remove(fullPath))
                    qDebug() << "Cancelled PDF deleted:" << fullPath;
                else
                    qDebug() << "Failed to delete cancelled PDF:" << fullPath;
            }
            processedFiles.removeOne(fullPath);
            continue;
        }

        QString printerName = dialog.getSelectedPrinter();
        bool manual = (dialog.getSelectedMode() == PrinterSelectionDialog::ManualDuplex);

        qDebug() << "Printing to:" << printerName << "Manual duplex:" << manual;

        Config::setPrinterName(printerName);

        bool success = PdfPrinter::printFile(fullPath, printerName, manual);

        if (success) {
            qDebug() << "Printed successfully:" << fullPath;

            if (Config::getDeleteAfterPrint()) {
                if (QFile::exists(fullPath)) {
                    if (QFile::remove(fullPath))
                        qDebug() << "PDF deleted after printing:" << fullPath;
                    else
                        qDebug() << "Failed to delete PDF after printing:" << fullPath;
                }
            }

            processedFiles.removeOne(fullPath);
        } else {
            qDebug() << "Printing failed:" << fullPath;
            processedFiles.removeOne(fullPath);
            QMessageBox::warning(mainWindow, "Print Error",
                                 "Failed to print the document. Please check your printer settings.");
        }
    }

    if (processedFiles.size() > 50)
        processedFiles.clear();

#else
    qDebug() << "Qt PDF support not available. Cannot process PDF files.";
#endif
}
