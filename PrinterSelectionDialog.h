#ifndef PRINTERSELECTIONDIALOG_H
#define PRINTERSELECTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class PrinterSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    enum PrintMode {
        None,
        ManualDuplex,
        Normal
    };

    explicit PrinterSelectionDialog(const QString &filename, const QStringList &printers, QWidget *parent = nullptr)
        : QDialog(parent), selectedMode(None)
    {
        setWindowTitle("QuantumPrint - Print Options");
        setModal(true);
        setMinimumWidth(400);

        setWindowIcon(QIcon(":/Logo.png"));

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *fileLabel = new QLabel(QString("New print job detected:\n%1").arg(filename), this);
        fileLabel->setWordWrap(true);
        mainLayout->addWidget(fileLabel);

        mainLayout->addSpacing(10);

        QLabel *printerLabel = new QLabel("Select Printer:", this);
        mainLayout->addWidget(printerLabel);

        printerCombo = new QComboBox(this);
        printerCombo->addItems(printers);
        mainLayout->addWidget(printerCombo);

        mainLayout->addSpacing(10);

        QLabel *modeLabel = new QLabel("Choose print mode:", this);
        mainLayout->addWidget(modeLabel);

        QHBoxLayout *buttonLayout = new QHBoxLayout();

        QPushButton *manualBtn = new QPushButton("Manual Duplex", this);
        QPushButton *normalBtn = new QPushButton("Normal (Single-sided)", this);
        QPushButton *cancelBtn = new QPushButton("Cancel", this);

        connect(manualBtn, &QPushButton::clicked, this, [this]() {
            selectedMode = ManualDuplex;
            accept();
        });

        connect(normalBtn, &QPushButton::clicked, this, [this]() {
            selectedMode = Normal;
            accept();
        });

        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

        buttonLayout->addWidget(manualBtn);
        buttonLayout->addWidget(normalBtn);
        buttonLayout->addWidget(cancelBtn);

        mainLayout->addSpacing(10);
        mainLayout->addLayout(buttonLayout);

        setLayout(mainLayout);
    }

    QString getSelectedPrinter() const {
        return printerCombo->currentText();
    }

    PrintMode getSelectedMode() const {
        return selectedMode;
    }

private:
    QComboBox *printerCombo;
    PrintMode selectedMode;
};

#endif
