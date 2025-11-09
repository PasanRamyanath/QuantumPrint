#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "LicenseDialog.h"
#include "LicenseManager.h"
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("QuantumPrint Printer Service");

    // Setup system tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/Logo.png"));
    trayIcon->setToolTip("QuantumPrint - Monitoring for print jobs");

    // Create tray menu
    QMenu *trayMenu = new QMenu(this);

    QAction *licenseInfoAction = new QAction("License Information", this);
    connect(licenseInfoAction, &QAction::triggered, this, &MainWindow::showLicenseInfo);

    QAction *changeLicenseAction = new QAction("Change License Key", this);
    connect(changeLicenseAction, &QAction::triggered, this, &MainWindow::changeLicense);

    QAction *aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    QAction *quitAction = new QAction("Exit", this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    trayMenu->addAction(licenseInfoAction);
    trayMenu->addAction(changeLicenseAction);
    trayMenu->addSeparator();
    trayMenu->addAction(aboutAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->setVisible(true);

    // Connect double-click to show window
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            show();
        }
    });

    // Start file watcher
    fileWatcher = new FileWatcher(this);
    fileWatcher->startWatching();

    // Show tray notification
    QString expDate = LicenseManager::getLicenseExpirationDate();
    trayIcon->showMessage("QuantumPrint Service Started",
                          QString("The printer monitoring service is now active.\nLicense valid until: %1").arg(expDate),
                          QSystemTrayIcon::Information,
                          3000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showAbout()
{
    QString expDate = LicenseManager::getLicenseExpirationDate();
    QMessageBox::about(this, "About QuantumPrint Printer Service",
                       "QuantumPrint v0.1\n\n"
                       "Manual Duplex Printing Service.\n\n"
                       "This application monitors for PDF files for printing "
                       "with duplex options.\n\n"
                       "License expires: " + expDate + "\n\n"
                                       "© 2025 Created By I.M Pasan Ramyanath\n\n"
                       "For inquiries, Contact :- impjr.business@gmail.com");
}

void MainWindow::showLicenseInfo()
{
    QString licenseKey = LicenseManager::getStoredLicenseKey();
    QString expDate = LicenseManager::getLicenseExpirationDate();
    QString hwId = LicenseManager::getHardwareId();

    // Mask the license key for security (show only first and last 5 chars)
    QString maskedKey = licenseKey;
    if (licenseKey.length() > 15) {
        maskedKey = licenseKey.left(11) + "***-***" + licenseKey.right(6);
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("License Information");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("QuantumPrint License Details");
    msgBox.setInformativeText(
        QString("License Key: %1\n"
                "Expiration Date: %2\n"
                "Hardware ID: %3\n\n"
                "Status: Active")
            .arg(maskedKey)
            .arg(expDate)
            .arg(hwId)
        );
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::changeLicense()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Change License Key",
        "Are you sure you want to change your license key?\n"
        "The current license will be deactivated.",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        LicenseManager::deactivateLicense();

        LicenseDialog licenseDialog(this);
        if (licenseDialog.exec() == QDialog::Accepted) {
            if (LicenseManager::isLicensed()) {
                QString expDate = LicenseManager::getLicenseExpirationDate();
                trayIcon->showMessage("License Updated",
                                      QString("New license activated successfully.\nValid until: %1").arg(expDate),
                                      QSystemTrayIcon::Information,
                                      3000);
            } else {
                QMessageBox::critical(this, "License Error",
                                      "Failed to activate new license. The application will now exit.");
                qApp->quit();
            }
        } else {
            // User cancelled, reactivate with old key if possible
            QMessageBox::warning(this, "License Required",
                                 "A valid license is required. The application will now exit.");
            qApp->quit();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}
