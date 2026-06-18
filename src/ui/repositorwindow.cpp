#include "repositorwindow.h"
#include "supplierdialog.h"
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QStatusBar>

RepositorWindow::RepositorWindow(const QString &username, int userId,
                                 QWidget *parent)
    : QMainWindow(parent), m_username(username), m_userId(userId) {
    setWindowTitle("Facturador — " + username);
    setAccessibleName("Panel de repositor");
    resize(950, 700);

    m_access = std::make_unique<RepositorAccess>();
    m_inventory = std::make_unique<InventoryController>(*m_access);
    m_suppliers = std::make_unique<SupplierController>(*m_access);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 8);
    mainLayout->setSpacing(8);

    tabs = new QTabWidget();
    tabs->setAccessibleName("Secciones del repositor");

    QWidget *tabProd = new QWidget();
    QVBoxLayout *layProd = new QVBoxLayout(tabProd);
    layProd->setContentsMargins(0, 8, 0, 0);
    setupProductsTab(layProd);
    tabs->addTab(tabProd, "Productos");

    QWidget *tabSup = new QWidget();
    QVBoxLayout *laySup = new QVBoxLayout(tabSup);
    laySup->setContentsMargins(0, 8, 0, 0);
    setupSuppliersTab(laySup);
    tabs->addTab(tabSup, "Proveedores");

    mainLayout->addWidget(tabs);

    QPushButton *btnLogout = new QPushButton("Cerrar sesi\u00f3n");
    btnLogout->setObjectName("btnText");
    btnLogout->setCursor(Qt::PointingHandCursor);
    connect(btnLogout, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Cerrar sesi\u00f3n",
                "\u00bfEst\u00e1 seguro de que desea cerrar sesi\u00f3n?",
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            emit loggedOut();
    });
    mainLayout->addWidget(btnLogout, 0, Qt::AlignCenter);

    setCentralWidget(central);

    statusBar()->showMessage(
        QString("Repositor: %1").arg(m_username));

    connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        switch (index) {
            case 0: loadProducts(); loadStockReport(); break;
            case 1: loadSuppliers(); break;
            default: break;
        }
    });

    loadProducts();
    loadStockReport();
}

void RepositorWindow::setupProductsTab(QVBoxLayout *layout) {
    QLabel *lblBienvenida =
        new QLabel(QString("Repositor: %1").arg(m_username));
    lblBienvenida->setStyleSheet("font-size: 18px; font-weight: 300; color: #1d1d1f;");
    layout->addWidget(lblBienvenida);

    QLabel *lblProd = new QLabel("Inventario de productos");
    lblProd->setStyleSheet("font-size: 14px; font-weight: 500; margin-top: 8px; color: #1d1d1f;");
    layout->addWidget(lblProd);

    tablaProductos = new QTableWidget(0, 6, this);
    tablaProductos->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Precio", "Stock", "Stock Minimo", "Proveedor"});
    tablaProductos->setAccessibleName("Inventario de productos");
    tablaProductos->horizontalHeader()->setStretchLastSection(true);
    tablaProductos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaProductos->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaProductos->setAlternatingRowColors(false);
    layout->addWidget(tablaProductos);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *btnActualizarStock = new QPushButton("Actualizar Stock");
    btnActualizarStock->setObjectName("btnPrimary");
    btnActualizarStock->setCursor(Qt::PointingHandCursor);
    connect(btnActualizarStock, &QPushButton::clicked, this,
            [this, btnActualizarStock]() {
        btnActualizarStock->setEnabled(false);
        updateStock();
        btnActualizarStock->setEnabled(true);
    });
    btnRow->addWidget(btnActualizarStock);

    QPushButton *btnRefresh = new QPushButton("Actualizar Productos");
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this,
            &RepositorWindow::loadProducts);
    btnRow->addWidget(btnRefresh);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    QLabel *lblStock = new QLabel("Productos con stock bajo");
    lblStock->setStyleSheet("font-size: 14px; font-weight: 500; margin-top: 12px; color: #1d1d1f;");
    layout->addWidget(lblStock);

    tablaStockBajo = new QTableWidget(0, 6, this);
    tablaStockBajo->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Stock Actual", "Stock Minimo", "Faltante", "Proveedor"});
    tablaStockBajo->setAccessibleName("Productos con stock bajo");
    tablaStockBajo->horizontalHeader()->setStretchLastSection(true);
    tablaStockBajo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaStockBajo->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaStockBajo->setAlternatingRowColors(false);
    layout->addWidget(tablaStockBajo);

    QHBoxLayout *bottomRow = new QHBoxLayout();
    QPushButton *btnRefreshStock = new QPushButton("Actualizar Reporte");
    btnRefreshStock->setCursor(Qt::PointingHandCursor);
    connect(btnRefreshStock, &QPushButton::clicked, this,
            &RepositorWindow::loadStockReport);
    bottomRow->addWidget(btnRefreshStock);
    bottomRow->addStretch();
    layout->addLayout(bottomRow);

    loadProducts();
    loadStockReport();
}

void RepositorWindow::setupSuppliersTab(QVBoxLayout *layout) {
    tablaProveedores = new QTableWidget(0, 6, this);
    tablaProveedores->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Contacto", "Telefono", "Email", "CUIT"});
    tablaProveedores->setAccessibleName("Lista de proveedores");
    tablaProveedores->horizontalHeader()->setStretchLastSection(true);
    tablaProveedores->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaProveedores->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaProveedores->setAlternatingRowColors(false);
    layout->addWidget(tablaProveedores);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Agregar Proveedor");
    btnAdd->setObjectName("btnPrimary");
    btnAdd->setCursor(Qt::PointingHandCursor);
    QPushButton *btnEdit = new QPushButton("Editar Proveedor");
    btnEdit->setCursor(Qt::PointingHandCursor);
    QPushButton *btnDel = new QPushButton("Eliminar Proveedor");
    btnDel->setObjectName("btnDanger");
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setToolTip("Elimina el proveedor seleccionado");
    connect(btnAdd, &QPushButton::clicked, this,
            [this, btnAdd]() {
        btnAdd->setEnabled(false);
        showAddSupplierDialog();
        btnAdd->setEnabled(true);
    });
    connect(btnEdit, &QPushButton::clicked, this,
            &RepositorWindow::showEditSupplierDialog);
    connect(btnDel, &QPushButton::clicked, this, &RepositorWindow::deleteSupplier);
    connect(tablaProveedores, &QTableWidget::cellDoubleClicked, this,
            [this](int, int) { showEditSupplierDialog(); });
    btnRow->addWidget(btnAdd);
    btnRow->addWidget(btnEdit);
    btnRow->addWidget(btnDel);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    loadSuppliers();
}

void RepositorWindow::loadProducts() {
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
            row, 4, new QTableWidgetItem(QString::number(p.minStock)));
        tablaProductos->setItem(
            row, 5, new QTableWidgetItem(p.supplierName));
    }
}

void RepositorWindow::loadStockReport() {
    tablaStockBajo->setRowCount(0);
    auto low = m_inventory->lowStockReport();
    for (const auto &p : low) {
        int row = tablaStockBajo->rowCount();
        tablaStockBajo->insertRow(row);
        tablaStockBajo->setItem(
            row, 0, new QTableWidgetItem(QString::number(p.id)));
        tablaStockBajo->setItem(row, 1, new QTableWidgetItem(p.name));
        tablaStockBajo->setItem(
            row, 2, new QTableWidgetItem(QString::number(p.stock)));
        tablaStockBajo->setItem(
            row, 3, new QTableWidgetItem(QString::number(p.minStock)));
        tablaStockBajo->setItem(
            row, 4,
            new QTableWidgetItem(QString::number(p.minStock - p.stock)));
        tablaStockBajo->setItem(
            row, 5, new QTableWidgetItem(p.supplierName));
    }
}

void RepositorWindow::loadSuppliers() {
    tablaProveedores->setRowCount(0);
    auto suppliers = m_suppliers->allSuppliers();
    for (const auto &s : suppliers) {
        int row = tablaProveedores->rowCount();
        tablaProveedores->insertRow(row);
        tablaProveedores->setItem(
            row, 0, new QTableWidgetItem(QString::number(s.id)));
        tablaProveedores->setItem(row, 1, new QTableWidgetItem(s.name));
        tablaProveedores->setItem(row, 2, new QTableWidgetItem(s.contact));
        tablaProveedores->setItem(row, 3, new QTableWidgetItem(s.phone));
        tablaProveedores->setItem(row, 4, new QTableWidgetItem(s.email));
        tablaProveedores->setItem(row, 5, new QTableWidgetItem(s.cuit));
    }
}

void RepositorWindow::updateStock() {
    int row = tablaProductos->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un producto");
        return;
    }

    auto *idItem = tablaProductos->item(row, 0);
    auto *nameItem = tablaProductos->item(row, 1);
    auto *stockItem = tablaProductos->item(row, 3);
    if (!idItem || !nameItem || !stockItem) {
        QMessageBox::warning(this, "Error", "Error al leer datos del producto");
        return;
    }
    int id = idItem->text().toInt();
    QString name = nameItem->text();
    int currentStock = stockItem->text().toInt();

    bool ok;
    int newStock = QInputDialog::getInt(this, "Actualizar Stock",
                                        "Nuevo stock para '" + name + "':",
                                        currentStock, 0, 999999, 1, &ok);
    if (!ok) return;

    auto result = m_inventory->findProduct(id);
    if (!result.success) {
        QMessageBox::warning(this, "Error", result.message);
        return;
    }

    Product p = result.value;
    p.stock = newStock;
    auto updateResult = m_inventory->updateProduct(p);
    if (updateResult.success) {
        loadProducts();
        loadStockReport();
        QMessageBox::information(this, "Stock actualizado",
                                QString("Stock de '%1' actualizado a %2")
                                    .arg(name).arg(newStock));
    } else {
        QMessageBox::warning(this, "Error", updateResult.errorMessage);
    }
}

void RepositorWindow::showAddSupplierDialog() {
    SupplierDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    Supplier s = dialog.supplier();
    auto result = m_suppliers->addSupplier(s.name, s.contact, s.phone,
                                           s.email, s.address, s.cuit);
    if (result.success)
        loadSuppliers();
    else
        QMessageBox::critical(this, "Error", result.errorMessage);
}

void RepositorWindow::showEditSupplierDialog() {
    int row = tablaProveedores->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un proveedor");
        return;
    }

    auto *idItem = tablaProveedores->item(row, 0);
    if (!idItem) {
        QMessageBox::warning(this, "Error", "Error al leer datos del proveedor");
        return;
    }
    int id = idItem->text().toInt();
    auto result = m_suppliers->findSupplier(id);
    if (!result.success) {
        QMessageBox::warning(this, "Error", result.message);
        return;
    }

    SupplierDialog dialog(result.value, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    Supplier s = dialog.supplier();
    s.id = id;
    auto updResult = m_suppliers->updateSupplier(s);
    if (updResult.success)
        loadSuppliers();
    else
        QMessageBox::critical(this, "Error", updResult.errorMessage);
}

void RepositorWindow::deleteSupplier() {
    int row = tablaProveedores->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un proveedor");
        return;
    }

    auto *idItem = tablaProveedores->item(row, 0);
    auto *nameItem = tablaProveedores->item(row, 1);
    if (!idItem || !nameItem) {
        QMessageBox::warning(this, "Error", "Error al leer datos del proveedor");
        return;
    }
    int id = idItem->text().toInt();
    QString name = nameItem->text();

    if (QMessageBox::question(this, "Confirmar",
                              "Eliminar proveedor '" + name + "'?",
                              QMessageBox::Yes | QMessageBox::No) ==
        QMessageBox::Yes) {
        auto result = m_suppliers->removeSupplier(id);
        if (result.success)
            loadSuppliers();
        else
            QMessageBox::warning(this, "Error", result.errorMessage);
    }
}
