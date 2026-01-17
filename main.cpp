#include "mainwindow.h"
#include "LicenseDialog.h"
#include "LicenseManager.h"
#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Logo.png"));

    QSharedMemory sharedMemory("QuantumPrint_SingleInstanceKey");
    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr, "Already Running",
                             "QuantumPrint Service is already running!");
        return 0;
    }

    if (!LicenseManager::isLicensed()) {
        LicenseDialog licenseDialog;
        if (licenseDialog.exec() != QDialog::Accepted) {
            QMessageBox::warning(nullptr, "License Required",
                                 "A valid license is required to use QuantumPrint.\n"
                                 "The application will now exit.");
            return 0;
        }
    }

    if (!LicenseManager::isLicensed()) {
        QMessageBox::critical(nullptr, "License Error",
                              "Failed to activate license. The application will now exit.");
        return 0;
    }

    MainWindow w;
    w.hide();

    return a.exec();
}
