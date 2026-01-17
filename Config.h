#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>
#include <QPrinterInfo>
#include <QStandardPaths>

class Config
{
public:
    static QString getWatchFolder() {
        QSettings settings("IMPJR", "IMPJR_Printer");
        return settings.value("WatchFolder", getDefaultWatchFolder()).toString();
    }

    static void setWatchFolder(const QString &path) {
        QSettings settings("IMPJR", "IMPJR_Printer");
        settings.setValue("WatchFolder", path);
    }

    static QString getPrinterName() {
        QSettings settings("IMPJR", "IMPJR_Printer");
        QString saved = settings.value("PrinterName").toString();

        if (saved.isEmpty() || !isPrinterValid(saved)) {
            QStringList printers = QPrinterInfo::availablePrinterNames();
            for (const QString &printer : printers) {
                if (!printer.contains("PDF", Qt::CaseInsensitive) &&
                    !printer.contains("XPS", Qt::CaseInsensitive) &&
                    !printer.contains("Fax", Qt::CaseInsensitive) &&
                    !printer.contains("OneNote", Qt::CaseInsensitive)) {
                    return printer;
                }
            }
            if (!printers.isEmpty()) {
                return printers.first();
            }
            return "";
        }
        return saved;
    }

    static void setPrinterName(const QString &name) {
        QSettings settings("IMPJR", "IMPJR_Printer");
        settings.setValue("PrinterName", name);
    }

    static bool getDeleteAfterPrint() {
        QSettings settings("IMPJR", "IMPJR_Printer");
        return settings.value("DeleteAfterPrint", true).toBool();
    }

    static void setDeleteAfterPrint(bool enable) {
        QSettings settings("IMPJR", "IMPJR_Printer");
        settings.setValue("DeleteAfterPrint", enable);
    }

    static int getCheckInterval() {
        QSettings settings("IMPJR", "IMPJR_Printer");
        return settings.value("CheckInterval", 3000).toInt();
    }

    static QStringList getAvailablePrinters() {
        return QPrinterInfo::availablePrinterNames();
    }

private:
    static QString getDefaultWatchFolder() {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        return documentsPath + "/CutePDF";
    }

    static bool isPrinterValid(const QString &name) {
        QStringList printers = QPrinterInfo::availablePrinterNames();
        return printers.contains(name);
    }
};

#endif
