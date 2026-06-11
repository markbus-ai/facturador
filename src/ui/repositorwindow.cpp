#include "repositorwindow.h"
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QStatusBar>

RepositorWindow::RepositorWindow(const QString &username, int userId,
                                 QWidget *parent)
    : QMainWindow(parent), m_username(username), m_userId(userId) {
    setWindowTitle("Facturador — " + username);
    setAccessibleName("Panel de repositor");
    resize(900, 650);

    m_access = std::make_unique<RepositorAccess>();
    m_inventory = std::make_unique<InventoryController>(*m_access);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 16, 16, 8);
    layout->setSpacing(8);

    QLabel *lblBienvenida =
        new QLabel(QString("Repositor: %1").arg(username));
    lblBienvenida->setStyleSheet("font-size: 18px; font-weight: 300; color: #1d1d1f;");
    layout->addWidget(lblBienvenida);

    QLabel *lblProd = new QLabel("Inventario de productos");
    lblProd->setStyleSheet("font-size: 14px; font-weight: 500; margin-top: 8px; color: #1d1d1f;");
    layout->addWidget(lblProd);

    tablaProductos = new QTableWidget(0, 5, this);
    tablaProductos->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Precio", "Stock", "Stock Minimo"});
    tablaProductos->setAccessibleName("Inventario de productos");
    tablaProductos->horizontalHeader()->setStretchLastSection(true);
    tablaProductos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaProductos->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaProductos->setAlternatingRowColors(true);
    layout->addWidget(tablaProductos);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *btnActualizarStock = new QPushButton("Actualizar Stock");
    btnActualizarStock->setObjectName("btnPrimary");
    btnActualizarStock->setCursor(Qt::PointingHandCursor);
    connect(btnActualizarStock, &QPushButton::clicked, this,
            &RepositorWindow::updateStock);
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

    tablaStockBajo = new QTableWidget(0, 5, this);
    tablaStockBajo->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Stock Actual", "Stock Minimo", "Faltante"});
    tablaStockBajo->setAccessibleName("Productos con stock bajo");
    tablaStockBajo->horizontalHeader()->setStretchLastSection(true);
    tablaStockBajo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaStockBajo->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaStockBajo->setAlternatingRowColors(true);
    layout->addWidget(tablaStockBajo);

    QHBoxLayout *bottomRow = new QHBoxLayout();
    QPushButton *btnRefreshStock = new QPushButton("Actualizar Reporte");
    btnRefreshStock->setCursor(Qt::PointingHandCursor);
    connect(btnRefreshStock, &QPushButton::clicked, this,
            &RepositorWindow::loadStockReport);
    bottomRow->addWidget(btnRefreshStock);

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
        QString("Repositor: %1").arg(m_username));

    loadProducts();
    loadStockReport();
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
    }
}

void RepositorWindow::updateStock() {
    int row = tablaProductos->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un producto");
        return;
    }

    int id = tablaProductos->item(row, 0)->text().toInt();
    QString name = tablaProductos->item(row, 1)->text();
    int currentStock = tablaProductos->item(row, 3)->text().toInt();

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
