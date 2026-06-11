#include "invoicedialog.h"
#include "services/InvoiceCalculator.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QFrame>
#include <QMetaObject>

InvoiceDialog::InvoiceDialog(BillingController &billing, int userId,
                             QWidget *parent)
    : QDialog(parent), m_billing(billing), m_userId(userId) {
    setWindowTitle("Nueva Factura");
    setAccessibleName("Nueva Factura");
    resize(860, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    QLabel *lblTitle = new QLabel("Nueva Factura");
    lblTitle->setStyleSheet("font-size: 20px; font-weight: 300; color: #1d1d1f;");
    mainLayout->addWidget(lblTitle);

    QHBoxLayout *clientRow = new QHBoxLayout();
    QLabel *lblCliente = new QLabel("Cliente");
    lblCliente->setStyleSheet("font-weight: 500; color: #6e6e73;");
    cmbCliente = new QComboBox();
    cmbCliente->setMinimumWidth(200);
    cmbCliente->setAccessibleName("Cliente");
    auto clients = m_billing.allClients();
    for (const auto &c : clients)
        cmbCliente->addItem(c.name, c.id);
    cmbCliente->setCurrentIndex(0);
    clientRow->addWidget(lblCliente);
    clientRow->addWidget(cmbCliente, 1);
    mainLayout->addLayout(clientRow);
    lblCliente->setBuddy(cmbCliente);

    QHBoxLayout *productRow = new QHBoxLayout();
    QLabel *lblProducto = new QLabel("Producto");
    lblProducto->setStyleSheet("font-weight: 500; color: #6e6e73;");
    cmbProducto = new QComboBox();
    cmbProducto->setAccessibleName("Producto");
    auto products = m_billing.allProducts();
    for (const auto &p : products) {
        cmbProducto->addItem(
            QString("%1 — $%2 (stock: %3)")
                .arg(p.name)
                .arg(p.price, 0, 'f', 2)
                .arg(p.stock),
            p.id);
    }
    productRow->addWidget(lblProducto);
    productRow->addWidget(cmbProducto, 1);
    lblProducto->setBuddy(cmbProducto);

    QPushButton *btnAddItem = new QPushButton("Agregar a Factura");
    btnAddItem->setObjectName("btnPrimary");
    btnAddItem->setCursor(Qt::PointingHandCursor);
    connect(btnAddItem, &QPushButton::clicked, this,
            &InvoiceDialog::addItemRow);
    productRow->addWidget(btnAddItem);
    mainLayout->addLayout(productRow);

    tablaItems = new QTableWidget(0, 7, this);
    tablaItems->setHorizontalHeaderLabels(
        {"Producto", "Cant.", "P. Unit.", "Subtotal",
         "Desc. Tipo", "Desc. Valor", "ID"});
    tablaItems->setAccessibleName("Items de la factura");
    tablaItems->horizontalHeader()->setStretchLastSection(true);
    tablaItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaItems->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaItems->setAlternatingRowColors(true);
    tablaItems->setColumnWidth(4, 90);
    tablaItems->setColumnWidth(5, 80);
    tablaItems->setColumnHidden(6, true);
    connect(tablaItems, &QTableWidget::currentCellChanged, this,
            [this](int, int, int, int) { syncItemDiscountControls(); });
    mainLayout->addWidget(tablaItems, 1);

    QHBoxLayout *itemBtnRow = new QHBoxLayout();
    QPushButton *btnRemove = new QPushButton("Quitar Item");
    btnRemove->setObjectName("btnDanger");
    btnRemove->setCursor(Qt::PointingHandCursor);
    connect(btnRemove, &QPushButton::clicked, this,
            &InvoiceDialog::removeItemRow);
    itemBtnRow->addWidget(btnRemove);
    itemBtnRow->addStretch();
    mainLayout->addLayout(itemBtnRow);

    grpDescuentoGlobal = new QGroupBox("Descuento global");
    QHBoxLayout *globalLay = new QHBoxLayout(grpDescuentoGlobal);
    cmbTipoDescuento = new QComboBox();
    cmbTipoDescuento->addItem("Sin descuento", "none");
    cmbTipoDescuento->addItem("Porcentaje (%)", "percentage");
    cmbTipoDescuento->addItem("Monto fijo ($)", "nominal");
    spnValorDescuento = new QDoubleSpinBox();
    spnValorDescuento->setRange(0, 999999);
    spnValorDescuento->setDecimals(2);
    spnValorDescuento->setValue(0);
    spnValorDescuento->setVisible(false);
    connect(cmbTipoDescuento, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                QString tipo = cmbTipoDescuento->itemData(idx).toString();
                spnValorDescuento->setVisible(tipo != "none");
                if (tipo == "percentage") {
                    spnValorDescuento->setRange(0, 100);
                    spnValorDescuento->setSuffix(" %");
                    spnValorDescuento->setDecimals(1);
                } else if (tipo == "nominal") {
                    spnValorDescuento->setRange(0, 999999);
                    spnValorDescuento->setPrefix("$ ");
                    spnValorDescuento->setSuffix("");
                    spnValorDescuento->setDecimals(2);
                }
                updateTotals();
            });
    connect(spnValorDescuento, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this]() { updateTotals(); });
    globalLay->addWidget(cmbTipoDescuento);
    globalLay->addWidget(spnValorDescuento);
    globalLay->addStretch();
    mainLayout->addWidget(grpDescuentoGlobal);

    grpDescuentoItem = new QGroupBox("Descuento por producto");
    QHBoxLayout *itemLay = new QHBoxLayout(grpDescuentoItem);
    cmbItemTipoDesc = new QComboBox();
    cmbItemTipoDesc->addItem("Sin descuento", "none");
    cmbItemTipoDesc->addItem("Porcentaje (%)", "percentage");
    cmbItemTipoDesc->addItem("Monto fijo ($)", "nominal");
    spnItemValorDesc = new QDoubleSpinBox();
    spnItemValorDesc->setRange(0, 999999);
    spnItemValorDesc->setDecimals(2);
    spnItemValorDesc->setValue(0);
    spnItemValorDesc->setVisible(false);
    connect(cmbItemTipoDesc, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                QString tipo = cmbItemTipoDesc->itemData(idx).toString();
                spnItemValorDesc->setVisible(tipo != "none");
                if (tipo == "percentage") {
                    spnItemValorDesc->setRange(0, 100);
                    spnItemValorDesc->setSuffix(" %");
                    spnItemValorDesc->setDecimals(1);
                } else if (tipo == "nominal") {
                    spnItemValorDesc->setRange(0, 999999);
                    spnItemValorDesc->setPrefix("$ ");
                    spnItemValorDesc->setSuffix("");
                    spnItemValorDesc->setDecimals(2);
                }
                int row = tablaItems->currentRow();
                if (row >= 0) {
                    tablaItems->item(row, 4)->setData(Qt::UserRole, tipo);
                    tablaItems->item(row, 5)->setText(
                        QString::number(spnItemValorDesc->value(), 'f', 2));
                    updateTotals();
                }
            });
    connect(spnItemValorDesc, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this]() {
                int row = tablaItems->currentRow();
                if (row >= 0) {
                    tablaItems->item(row, 5)->setText(
                        QString::number(spnItemValorDesc->value(), 'f', 2));
                    updateTotals();
                }
            });
    itemLay->addWidget(cmbItemTipoDesc);
    itemLay->addWidget(spnItemValorDesc);
    itemLay->addStretch();
    mainLayout->addWidget(grpDescuentoItem);

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background: #e5e5ea; max-height: 1px;");
    mainLayout->addWidget(separator);

    QHBoxLayout *totalsRow = new QHBoxLayout();
    totalsRow->setSpacing(24);

    lblSubtotal = new QLabel("Subtotal: $0.00");
    lblDescuento = new QLabel("Descuento: $0.00");
    lblVat = new QLabel("IVA (21%): $0.00");
    lblTotal = new QLabel("Total: $0.00");

    lblSubtotal->setStyleSheet("font-size: 13px; color: #6e6e73;");
    lblDescuento->setStyleSheet("font-size: 13px; color: #d92b20;");
    lblVat->setStyleSheet("font-size: 13px; color: #6e6e73;");
    lblTotal->setStyleSheet("font-size: 18px; font-weight: 600; color: #1d1d1f;");

    totalsRow->addWidget(lblSubtotal);
    totalsRow->addWidget(lblDescuento);
    totalsRow->addWidget(lblVat);
    totalsRow->addWidget(lblTotal);
    totalsRow->addStretch();
    mainLayout->addLayout(totalsRow);

    mainLayout->addSpacing(8);

    QHBoxLayout *actionRow = new QHBoxLayout();
    actionRow->addStretch();
    QPushButton *btnGuardar = new QPushButton("Guardar Factura");
    btnGuardar->setObjectName("btnPrimary");
    btnGuardar->setCursor(Qt::PointingHandCursor);
    btnGuardar->setMinimumWidth(140);
    QPushButton *btnCancelar = new QPushButton("Cancelar");
    btnCancelar->setCursor(Qt::PointingHandCursor);
    connect(btnGuardar, &QPushButton::clicked, this,
            &InvoiceDialog::saveInvoice);
    connect(btnCancelar, &QPushButton::clicked, this, [this]() {
        if (tablaItems->rowCount() > 0 &&
            QMessageBox::question(this, "Cancelar",
                "Se perder\u00e1n los items agregados. \u00bfCancelar factura?",
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;
        reject();
    });
    actionRow->addWidget(btnGuardar);
    actionRow->addWidget(btnCancelar);
    mainLayout->addLayout(actionRow);

    syncItemDiscountControls();
}

void InvoiceDialog::addItemRow() {
    int prodId = cmbProducto->currentData().toInt();
    auto products = m_billing.allProducts();
    Product selected;
    bool found = false;
    for (const auto &p : products) {
        if (p.id == prodId) {
            selected = p;
            found = true;
            break;
        }
    }
    if (!found) return;

    disconnect(m_cellChangedConn);

    int row = tablaItems->rowCount();
    tablaItems->insertRow(row);

    int col = 0;
    tablaItems->setItem(row, col++, new QTableWidgetItem(selected.name));
    QTableWidgetItem *qtyItem = new QTableWidgetItem("1");
    qtyItem->setFlags(qtyItem->flags() | Qt::ItemIsEditable);
    tablaItems->setItem(row, col++, qtyItem);
    tablaItems->setItem(row, col++,
        new QTableWidgetItem(QString("$%1").arg(selected.price, 0, 'f', 2)));
    tablaItems->setItem(row, col++,
        new QTableWidgetItem(QString("$%1").arg(selected.price, 0, 'f', 2)));
    QTableWidgetItem *tipoItem = new QTableWidgetItem();
    tipoItem->setData(Qt::UserRole, "none");
    tablaItems->setItem(row, col++, tipoItem);
    tablaItems->setItem(row, col++, new QTableWidgetItem("0"));
    tablaItems->setItem(row, col++,
        new QTableWidgetItem(QString::number(selected.id)));

    m_cellChangedConn = connect(tablaItems, &QTableWidget::cellChanged, this,
            [this](int r, int c) {
                if (c == 1) updateTotals();
            });

    tablaItems->setCurrentCell(row, 0);
    syncItemDiscountControls();
    updateTotals();
}

void InvoiceDialog::removeItemRow() {
    int row = tablaItems->currentRow();
    if (row >= 0 && tablaItems->rowCount() > 0) {
        disconnect(m_cellChangedConn);
        tablaItems->removeRow(row);
        m_cellChangedConn = connect(tablaItems, &QTableWidget::cellChanged, this,
                [this](int r, int c) {
                    if (c == 1) updateTotals();
                });
        syncItemDiscountControls();
        updateTotals();
    }
}

void InvoiceDialog::syncItemDiscountControls() {
    int row = tablaItems->currentRow();
    if (row < 0) {
        cmbItemTipoDesc->setEnabled(false);
        spnItemValorDesc->setEnabled(false);
        return;
    }
    cmbItemTipoDesc->setEnabled(true);
    spnItemValorDesc->setEnabled(true);

    QString tipo = tablaItems->item(row, 4)->data(Qt::UserRole).toString();
    double valor = tablaItems->item(row, 5)->text().toDouble();

    int idx = cmbItemTipoDesc->findData(tipo);
    cmbItemTipoDesc->blockSignals(true);
    cmbItemTipoDesc->setCurrentIndex(idx >= 0 ? idx : 0);
    cmbItemTipoDesc->blockSignals(false);

    spnItemValorDesc->blockSignals(true);
    spnItemValorDesc->setValue(valor);
    spnItemValorDesc->setVisible(tipo != "none");
    if (tipo == "percentage") {
        spnItemValorDesc->setRange(0, 100);
        spnItemValorDesc->setSuffix(" %");
    } else if (tipo == "nominal") {
        spnItemValorDesc->setRange(0, 999999);
        spnItemValorDesc->setPrefix("$ ");
        spnItemValorDesc->setSuffix("");
    }
    spnItemValorDesc->blockSignals(false);
}

void InvoiceDialog::updateTotals() {
    double subtotalSinDesc = 0.0;
    double sumConDesc = 0.0;

    for (int i = 0; i < tablaItems->rowCount(); i++) {
        int qty = tablaItems->item(i, 1)->text().toInt();
        if (qty < 1) qty = 1;
        QString ps = tablaItems->item(i, 2)->text();
        ps.remove('$');
        double price = ps.toDouble();
        double lineTotal = qty * price;
        subtotalSinDesc += lineTotal;

        QString tipo = tablaItems->item(i, 4)->data(Qt::UserRole).toString();
        double descValor = tablaItems->item(i, 5)->text().toDouble();
        double finalLine = lineTotal;
        if (tipo == "percentage")
            finalLine = lineTotal * (1.0 - descValor / 100.0);
        else if (tipo == "nominal")
            finalLine = std::max(0.0, lineTotal - descValor);

        sumConDesc += finalLine;
    }

    auto global = crearEstrategiaGlobal();
    InvoiceTotals t;
    t.subtotal = subtotalSinDesc;
    double porItem = subtotalSinDesc - sumConDesc;
    double globalDisc = global->calculateDiscount(sumConDesc);
    t.discountAmount = porItem + globalDisc;
    t.taxableAmount = sumConDesc - globalDisc;
    t.vat = t.taxableAmount * InvoiceCalculator::IVA_RATE;
    t.total = t.taxableAmount + t.vat;

    lblSubtotal->setText(
        QString("Subtotal: $%1").arg(t.subtotal, 0, 'f', 2));
    QString descText = QString("Descuento: -$%1").arg(t.discountAmount, 0, 'f', 2);
    if (porItem > 0 && globalDisc > 0)
        descText += QString(" (item: $%1, global: $%2)").arg(porItem, 0, 'f', 2).arg(globalDisc, 0, 'f', 2);
    else if (porItem > 0)
        descText += " (por producto)";
    else if (globalDisc > 0)
        descText += " (global)";
    lblDescuento->setText(descText);
    lblVat->setText(
        QString("IVA (21%%): $%1").arg(t.vat, 0, 'f', 2));
    lblTotal->setText(
        QString("Total: $%1").arg(t.total, 0, 'f', 2));
}

std::unique_ptr<IDiscountStrategy> InvoiceDialog::crearEstrategiaGlobal() const {
    QString tipo = cmbTipoDescuento->currentData().toString();
    double valor = spnValorDescuento->value();
    return DiscountFactory::create(tipo, valor);
}

void InvoiceDialog::saveInvoice() {
    if (tablaItems->rowCount() == 0) {
        QMessageBox::warning(this, "Error", "Agregue al menos un producto");
        return;
    }

    QList<InvoiceItem> items;
    for (int i = 0; i < tablaItems->rowCount(); i++) {
        InvoiceItem item;
        item.productId = tablaItems->item(i, 6)->text().toInt();
        item.productName = tablaItems->item(i, 0)->text();
        item.quantity = tablaItems->item(i, 1)->text().toInt();

        if (item.quantity <= 0) {
            QMessageBox::warning(this, "Error",
                "La cantidad de '" + item.productName + "' debe ser mayor a cero");
            return;
        }

        QString priceStr = tablaItems->item(i, 2)->text();
        priceStr.remove('$');
        item.unitPrice = priceStr.toDouble();
        item.subtotal = item.quantity * item.unitPrice;

        item.discountType = tablaItems->item(i, 4)->data(Qt::UserRole).toString();
        item.discountValue = tablaItems->item(i, 5)->text().toDouble();

        items.append(item);
    }

    auto discount = crearEstrategiaGlobal();

    auto result = m_billing.submitInvoice(
        cmbCliente->currentData().toInt(), m_userId, items, std::move(discount));

    if (result.success) {
        QMessageBox::information(this, "Exito", "Factura creada correctamente");
        accept();
    } else {
        QMessageBox::warning(this, "Error", result.errorMessage);
    }
}
