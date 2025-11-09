#ifndef PDFPRINTER_H
#define PDFPRINTER_H

#include <QString>

class PdfPrinter
{
public:
    static bool printFile(const QString &filePath, const QString &printerName, bool manualDuplex);
};

#endif // PDFPRINTER_H
