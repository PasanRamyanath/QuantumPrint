#include "PdfPrinter.h"
#include <QPdfDocument>
#include <QPrinter>
#include <QPainter>
#include <QMessageBox>
#include <QPrinterInfo>
#include <QPageSize>
#include <QDebug>
#include <algorithm>

bool PdfPrinter::printFile(const QString &filePath, const QString &printerName, bool manualDuplex)
{
    qDebug() << "Starting print job for:" << filePath;

    QPdfDocument pdf;
    auto loadResult = pdf.load(filePath);
    if (loadResult != QPdfDocument::Error::None) {
        qDebug() << "Failed to load PDF. Error code:" << static_cast<int>(loadResult);
        QMessageBox::warning(nullptr, "Error", "Failed to load PDF: " + filePath);
        return false;
    }

    int totalPages = pdf.pageCount();
    qDebug() << "PDF loaded successfully. Pages:" << totalPages;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPrinterName(printerName);
    if (!printer.isValid()) {
        QString availablePrinters;
        for (const QString &p : QPrinterInfo::availablePrinterNames())
            availablePrinters += "\n- " + p;
        QMessageBox::warning(nullptr, "Printer Error",
                             QString("Printer '%1' is not valid.\nAvailable printers:%2")
                                 .arg(printerName, availablePrinters));
        return false;
    }

    printer.setOutputFormat(QPrinter::NativeFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setResolution(300);
    QSizeF pdfPageSizeMM = pdf.pagePointSize(0) * 0.352778; // points → mm
    printer.setPageSize(QPageSize(pdfPageSizeMM, QPageSize::Millimeter));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setFullPage(true);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::warning(nullptr, "Error", "Failed to begin printing.");
        return false;
    }

    auto printPages = [&](const QList<int> &pages) -> bool {
        bool firstPage = true;
        for (int pageIndex : pages) {
            if (!firstPage && !printer.newPage()) {
                qDebug() << "Failed to create new page";
                return false;
            }
            firstPage = false;

            QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
            QSizeF pdfPageSize = pdf.pagePointSize(pageIndex);
            int dpi = printer.resolution();
            QSize renderSize(pdfPageSize.width() * dpi / 72.0, pdfPageSize.height() * dpi / 72.0);

            QImage image = pdf.render(pageIndex, renderSize);
            if (image.isNull()) {
                qDebug() << "Failed to render page" << (pageIndex + 1);
                continue;
            }

            // Draw the PDF content first
            painter.drawImage(pageRect, image);

            // Draw diagonal line on top of the content
            QPen pen(QColor(250, 250, 250));
            pen.setWidth(1);
            painter.setPen(pen);
            painter.drawLine(pageRect.topLeft(), pageRect.bottomRight());
        }
        return true;
    };

    bool success = true;

    if (manualDuplex && totalPages > 1) {
        QList<int> oddPages, evenPages;
        for (int i = 0; i < totalPages; ++i) {
            if (i % 2 == 0)
                oddPages.append(i);
            else
                evenPages.append(i);
        }

        qDebug() << "Printing odd pages...";
        if (!printPages(oddPages)) success = false;
        painter.end();

        QMessageBox msgBox;
        msgBox.setWindowTitle("Manual Duplex - Flip Pages");
        msgBox.setText(QString("Odd pages printed (%1 pages).\n\n"
                               "Please:\n"
                               "1. Remove the printed pages from output tray\n"
                               "2. Flip them over (rotate 180°)\n"
                               "3. Place them back in the input tray\n"
                               "4. Click OK to print even pages")
                           .arg(oddPages.size()));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        if (msgBox.exec() == QMessageBox::Ok) {
            if (!painter.begin(&printer)) {
                QMessageBox::warning(nullptr, "Error", "Failed to begin printing even pages.");
                return false;
            }

            bool hasOddPageCount = (totalPages % 2 != 0);
            std::reverse(evenPages.begin(), evenPages.end());
            qDebug() << "Printing even pages in reverse" << (hasOddPageCount ? "with blank page first" : "");

            if (hasOddPageCount) {
                qDebug() << "Document has odd page count — printing a forced blank page first.";
                QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();

                QImage blankImage(pageRect.size(), QImage::Format_RGB32);
                blankImage.fill(Qt::white);

                QPainter imgPainter(&blankImage);
                QPen pen(QColor(250, 250, 250));
                pen.setWidth(1);
                imgPainter.setPen(pen);
                imgPainter.drawLine(0, 0, pageRect.width(), pageRect.height());
                imgPainter.end();

                painter.drawImage(pageRect, blankImage);
                painter.end();

                if (!painter.begin(&printer)) {
                    qDebug() << "Failed to restart painter after blank page.";
                    success = false;
                } else {
                    qDebug() << "Blank page sent successfully before even pages.";
                }
            }

            bool firstPage = true;
            for (int pageIndex : evenPages) {
                if (!firstPage && !printer.newPage()) {
                    qDebug() << "Failed to create new page for even side.";
                    success = false;
                    break;
                }
                firstPage = false;

                QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
                QSizeF pdfPageSize = pdf.pagePointSize(pageIndex);
                int dpi = printer.resolution();
                QSize renderSize(pdfPageSize.width() * dpi / 72.0, pdfPageSize.height() * dpi / 72.0);

                QImage image = pdf.render(pageIndex, renderSize);
                if (image.isNull()) {
                    qDebug() << "Failed to render even page" << (pageIndex + 1);
                    continue;
                }

                // Draw the PDF content first
                painter.drawImage(pageRect, image);

                // Draw diagonal line on top of the content
                QPen pen(QColor(250, 250, 250));
                pen.setWidth(1);
                painter.setPen(pen);
                painter.drawLine(pageRect.topLeft(), pageRect.bottomRight());
            }
        }

    } else {
        QList<int> allPages;
        for (int i = 0; i < totalPages; ++i) allPages.append(i);
        if (!printPages(allPages)) success = false;
    }

    painter.end();
    pdf.close();

    if (success) {
        QMessageBox::information(nullptr, "Print Complete",
                                 QString("Document printed successfully!\n%1 pages sent to %2")
                                     .arg(totalPages)
                                     .arg(printerName));
    }

    return success;
}
