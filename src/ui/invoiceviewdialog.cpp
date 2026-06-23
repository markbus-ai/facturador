#include "invoiceviewdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QFrame>

InvoiceViewDialog::InvoiceViewDialog(const Invoice &invoice,
                                     QWidget *parent)
    : QDialog(parent), m_invoice(invoice) {
    setWindowTitle(QString("Factura #%1").arg(m_invoice.id));
    setMinimumSize(700, 500);
    resize(750, 550);
    setupUi();
}

void InvoiceViewDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 16);

    QGroupBox *grpHeader = new QGroupBox("Informacion de la Factura");
    QFormLayout *form = new QFormLayout(grpHeader);

    form->addRow("N° Factura:", new QLabel(QString::number(m_invoice.id)));
    form->addRow("Cliente:", new QLabel(m_invoice.clientName));
    form->addRow("Usuario:", new QLabel(m_invoice.userName));
    form->addRow("Fecha:", new QLabel(m_invoice.date));
    mainLayout->addWidget(grpHeader);

    QLabel *lblItemsTitle = new QLabel("Detalle de Items");
    lblItemsTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #1d1d1f;");
    mainLayout->addWidget(lblItemsTitle);

    tablaItems = new QTableWidget(0, 6, this);
    tablaItems->setHorizontalHeaderLabels(
        {"Producto", "Cantidad", "P. Unitario", "Subtotal", "Dto.", "Total"});
    tablaItems->horizontalHeader()->setStretchLastSection(true);
    tablaItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaItems->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaItems->setAlternatingRowColors(false);
    mainLayout->addWidget(tablaItems);

    for (const auto &item : m_invoice.items) {
        int row = tablaItems->rowCount();
        tablaItems->insertRow(row);
        tablaItems->setItem(row, 0, new QTableWidgetItem(item.productName));
        tablaItems->setItem(row, 1,
            new QTableWidgetItem(QString::number(item.quantity)));
        tablaItems->setItem(row, 2,
            new QTableWidgetItem(QString("$%1").arg(item.unitPrice, 0, 'f', 2)));
        tablaItems->setItem(row, 3,
            new QTableWidgetItem(QString("$%1").arg(item.subtotal, 0, 'f', 2)));

        QString descStr;
        if (item.discountType == "percentage")
            descStr = QString("%1%").arg(item.discountValue, 0, 'f', 0);
        else if (item.discountType == "nominal")
            descStr = QString("$%1").arg(item.discountValue, 0, 'f', 2);
        else
            descStr = "-";
        tablaItems->setItem(row, 4, new QTableWidgetItem(descStr));

        tablaItems->setItem(row, 5,
            new QTableWidgetItem(
                QString("$%1").arg(item.discountedSubtotal(), 0, 'f', 2)));
    }

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);

    QGroupBox *grpTotals = new QGroupBox("Totales");
    QFormLayout *totForm = new QFormLayout(grpTotals);

    QString subtotalStr = QString("$%1").arg(m_invoice.subtotal, 0, 'f', 2);
    totForm->addRow("Subtotal:", new QLabel(subtotalStr));

    QString discStr = QString("$%1").arg(m_invoice.discountAmount, 0, 'f', 2);
    if (!m_invoice.discountStrategyName.isEmpty())
        discStr += QString(" (%1)").arg(m_invoice.discountStrategyName);
    totForm->addRow("Descuento:", new QLabel(discStr));

    QString vatStr = QString("$%1").arg(m_invoice.vat, 0, 'f', 2);
    totForm->addRow("IVA (21%):", new QLabel(vatStr));

    QLabel *lblTotal = new QLabel(
        QString("$%1").arg(m_invoice.total, 0, 'f', 2));
    lblTotal->setStyleSheet("font-size: 16px; font-weight: 700; color: #1d1d1f;");
    totForm->addRow("Total:", lblTotal);

    mainLayout->addWidget(grpTotals);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    QPushButton *btnCerrar = new QPushButton("Cerrar");
    btnCerrar->setObjectName("btnPrimary");
    btnCerrar->setCursor(Qt::PointingHandCursor);
    connect(btnCerrar, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(btnCerrar);
    mainLayout->addLayout(btnRow);
}
