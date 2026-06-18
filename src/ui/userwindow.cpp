#include "userwindow.h"
#include "invoicedialog.h"
#include <QHeaderView>
#include <QStatusBar>
#include <QMessageBox>

UserWindow::UserWindow(const QString &username, int userId,
                       QWidget *parent)
    : QMainWindow(parent), m_username(username), m_userId(userId) {
    setWindowTitle("Facturador — " + username);
    setAccessibleName("Panel de usuario");
    resize(900, 600);

    m_access = std::make_unique<UserAccess>();
    m_billing = std::make_unique<BillingController>(*m_access, m_userId);
    m_inventory = std::make_unique<InventoryController>(*m_access);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 16, 16, 8);
    layout->setSpacing(8);

    QLabel *lblBienvenida =
        new QLabel(QString("Bienvenido, %1").arg(username));
    lblBienvenida->setStyleSheet("font-size: 18px; font-weight: 300; color: #1d1d1f;");
    layout->addWidget(lblBienvenida);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnNuevaFactura = new QPushButton("Nueva Factura");
    btnNuevaFactura->setObjectName("btnPrimary");
    btnNuevaFactura->setCursor(Qt::PointingHandCursor);
    connect(btnNuevaFactura, &QPushButton::clicked, this,
            [this]() {
        btnNuevaFactura->setEnabled(false);
        showNewInvoiceDialog();
        btnNuevaFactura->setEnabled(true);
    });
    btnRow->addWidget(btnNuevaFactura);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    tablaFacturas = new QTableWidget(0, 5, this);
    tablaFacturas->setHorizontalHeaderLabels(
        {"ID", "Cliente", "Fecha", "Total", "Items"});
    tablaFacturas->setAccessibleName("Mis facturas");
    tablaFacturas->horizontalHeader()->setStretchLastSection(true);
    tablaFacturas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaFacturas->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaFacturas->setAlternatingRowColors(false);
    layout->addWidget(tablaFacturas);

    QLabel *lblProd = new QLabel("Productos disponibles");
    lblProd->setStyleSheet("font-size: 14px; font-weight: 500; margin-top: 12px; color: #1d1d1f;");
    layout->addWidget(lblProd);

    tablaProductos = new QTableWidget(0, 5, this);
    tablaProductos->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Precio", "Stock", "Proveedor"});
    tablaProductos->setAccessibleName("Productos disponibles");
    tablaProductos->horizontalHeader()->setStretchLastSection(true);
    tablaProductos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaProductos->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaProductos->setAlternatingRowColors(false);
    layout->addWidget(tablaProductos);

    QHBoxLayout *bottomRow = new QHBoxLayout();

    QPushButton *btnRefreshProds = new QPushButton("Actualizar Productos");
    btnRefreshProds->setCursor(Qt::PointingHandCursor);
    connect(btnRefreshProds, &QPushButton::clicked, this,
            &UserWindow::loadProducts);
    bottomRow->addWidget(btnRefreshProds);

    bottomRow->addStretch();

    QPushButton *btnLogout = new QPushButton("Cerrar sesi\u00f3n");
    btnLogout->setObjectName("btnText");
    btnLogout->setCursor(Qt::PointingHandCursor);
    connect(btnLogout, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Cerrar sesi\u00f3n",
                "\u00bfEst\u00e1 seguro de que desea cerrar sesi\u00f3n?",
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            emit loggedOut();
    });
    bottomRow->addWidget(btnLogout);

    layout->addLayout(bottomRow);

    setCentralWidget(central);

    statusBar()->showMessage(
        QString("Usuario: %1").arg(m_username));

    loadInvoices();
    loadProducts();
}

void UserWindow::loadInvoices() {
    tablaFacturas->setRowCount(0);
    auto invoices = m_billing->myInvoices();
    for (const auto &inv : invoices) {
        int row = tablaFacturas->rowCount();
        tablaFacturas->insertRow(row);
        tablaFacturas->setItem(
            row, 0, new QTableWidgetItem(QString::number(inv.id)));
        tablaFacturas->setItem(row, 1,
                               new QTableWidgetItem(inv.clientName));
        tablaFacturas->setItem(row, 2, new QTableWidgetItem(inv.date));
        tablaFacturas->setItem(
            row, 3,
            new QTableWidgetItem(
                QString("$%1").arg(inv.total, 0, 'f', 2)));
        tablaFacturas->setItem(
            row, 4,
            new QTableWidgetItem(QString::number(inv.items.size())));
    }
}

void UserWindow::loadProducts() {
    tablaProductos->setRowCount(0);
    auto products = m_inventory->allProducts();
    for (const auto &p : products) {
        int row = tablaProductos->rowCount();
        tablaProductos->insertRow(row);
        tablaProductos->setItem(
            row, 0, new QTableWidgetItem(QString::number(p.id)));
        tablaProductos->setItem(row, 1, new QTableWidgetItem(p.name));
        tablaProductos->setItem(
            row, 2,
            new QTableWidgetItem(
                QString("$%1").arg(p.price, 0, 'f', 2)));
        tablaProductos->setItem(
            row, 3, new QTableWidgetItem(QString::number(p.stock)));
        tablaProductos->setItem(
            row, 4, new QTableWidgetItem(p.supplierName));
    }
}

void UserWindow::showNewInvoiceDialog() {
    InvoiceDialog dialog(*m_billing, m_userId, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadInvoices();
        loadProducts();
    }
}
