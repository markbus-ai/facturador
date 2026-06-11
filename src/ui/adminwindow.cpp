#include "adminwindow.h"
#include "invoicedialog.h"
#include "controllers/AuthController.h"
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QComboBox>

AdminWindow::AdminWindow(const QString &username, int userId,
                         QWidget *parent)
    : QMainWindow(parent), m_username(username), m_userId(userId) {
    setWindowTitle("Facturador — Administrador");
    setAccessibleName("Panel de administraci\u00f3n");
    resize(1100, 750);

    m_access = std::make_unique<AdminAccess>();
    m_inventory = std::make_unique<InventoryController>(*m_access);
    m_billing = std::make_unique<BillingController>(*m_access, m_userId);
    m_clients = std::make_unique<ClientController>(*m_access);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 8);
    mainLayout->setSpacing(8);

    tabs = new QTabWidget();
    tabs->setAccessibleName("Secciones de administraci\u00f3n");

    QWidget *tabFact = new QWidget();
    QVBoxLayout *layFact = new QVBoxLayout(tabFact);
    layFact->setContentsMargins(0, 8, 0, 0);
    setupInvoicesTab(layFact);
    tabs->addTab(tabFact, "Facturas");

    QWidget *tabProd = new QWidget();
    QVBoxLayout *layProd = new QVBoxLayout(tabProd);
    layProd->setContentsMargins(0, 8, 0, 0);
    setupProductsTab(layProd);
    tabs->addTab(tabProd, "Productos");

    QWidget *tabCli = new QWidget();
    QVBoxLayout *layCli = new QVBoxLayout(tabCli);
    layCli->setContentsMargins(0, 8, 0, 0);
    setupClientsTab(layCli);
    tabs->addTab(tabCli, "Clientes");

    QWidget *tabUsr = new QWidget();
    QVBoxLayout *layUsr = new QVBoxLayout(tabUsr);
    layUsr->setContentsMargins(0, 8, 0, 0);
    setupUsersTab(layUsr);
    tabs->addTab(tabUsr, "Usuarios");

    QWidget *tabStock = new QWidget();
    QVBoxLayout *layStock = new QVBoxLayout(tabStock);
    layStock->setContentsMargins(0, 8, 0, 0);
    setupStockReportTab(layStock);
    tabs->addTab(tabStock, "Stock Bajo");

    mainLayout->addWidget(tabs);

    connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        switch (index) {
            case 0: loadInvoices(); break;
            case 1: loadProducts(); break;
            case 2: loadClients(); break;
            case 3: loadUsers(); break;
            case 4: loadStockReport(); break;
        }
    });

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
        QString("Administrador: %1").arg(m_username));
}

void AdminWindow::setupInvoicesTab(QVBoxLayout *layout) {
    QLabel *lbl = new QLabel(
        QString("Administrador: %1").arg(m_username));
    lbl->setStyleSheet("font-size: 18px; font-weight: 300; color: #1d1d1f;");
    layout->addWidget(lbl);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *btnNueva = new QPushButton("Nueva Factura");
    btnNueva->setObjectName("btnPrimary");
    btnNueva->setCursor(Qt::PointingHandCursor);
    connect(btnNueva, &QPushButton::clicked, this,
            &AdminWindow::showNewInvoiceDialog);
    btnRow->addWidget(btnNueva);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    tablaFacturas = new QTableWidget(0, 6, this);
    tablaFacturas->setHorizontalHeaderLabels(
        {"ID", "Cliente", "Usuario", "Fecha", "Total", "Items"});
    tablaFacturas->setAccessibleName("Lista de facturas");
    tablaFacturas->horizontalHeader()->setStretchLastSection(true);
    tablaFacturas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaFacturas->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaFacturas->setAlternatingRowColors(true);
    layout->addWidget(tablaFacturas);

    QPushButton *btnEliminar = new QPushButton("Eliminar factura seleccionada");
    btnEliminar->setObjectName("btnDanger");
    btnEliminar->setCursor(Qt::PointingHandCursor);
    btnEliminar->setToolTip("Elimina la factura seleccionada de la lista");
    connect(btnEliminar, &QPushButton::clicked, this, [this]() {
        int row = tablaFacturas->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Error", "Seleccione una factura");
            return;
        }
        int id = tablaFacturas->item(row, 0)->text().toInt();
        QString clientName = tablaFacturas->item(row, 1)->text();
        QString total = tablaFacturas->item(row, 4)->text();
        if (QMessageBox::question(this, "Confirmar",
                                  QString("Eliminar factura #%1?\nCliente: %2\nTotal: %3")
                                      .arg(id).arg(clientName).arg(total),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            auto result = m_billing->deleteInvoice(id);
            if (result.success)
                loadInvoices();
            else
                QMessageBox::warning(this, "Error", result.errorMessage);
        }
    });
    layout->addWidget(btnEliminar);

    loadInvoices();
}

void AdminWindow::setupClientsTab(QVBoxLayout *layout) {
    tablaClientes = new QTableWidget(0, 4, this);
    tablaClientes->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Direccion", "Telefono"});
    tablaClientes->setAccessibleName("Lista de clientes");
    tablaClientes->horizontalHeader()->setStretchLastSection(true);
    tablaClientes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaClientes->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaClientes->setAlternatingRowColors(true);
    layout->addWidget(tablaClientes);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Agregar Cliente");
    btnAdd->setObjectName("btnPrimary");
    btnAdd->setCursor(Qt::PointingHandCursor);
    QPushButton *btnEdit = new QPushButton("Editar Cliente");
    btnEdit->setCursor(Qt::PointingHandCursor);
    QPushButton *btnDel = new QPushButton("Eliminar Cliente");
    btnDel->setObjectName("btnDanger");
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setToolTip("Elimina el cliente seleccionado");
    connect(btnAdd, &QPushButton::clicked, this,
            &AdminWindow::showAddClientDialog);
    connect(btnEdit, &QPushButton::clicked, this,
            &AdminWindow::showEditClientDialog);
    connect(btnDel, &QPushButton::clicked, this, &AdminWindow::deleteClient);
    connect(tablaClientes, &QTableWidget::cellDoubleClicked, this,
            [this](int, int) { showEditClientDialog(); });
    btnRow->addWidget(btnAdd);
    btnRow->addWidget(btnEdit);
    btnRow->addWidget(btnDel);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    loadClients();
}

void AdminWindow::setupUsersTab(QVBoxLayout *layout) {
    tablaUsuarios = new QTableWidget(0, 3, this);
    tablaUsuarios->setHorizontalHeaderLabels({"ID", "Usuario", "Rol"});
    tablaUsuarios->setAccessibleName("Lista de usuarios");
    tablaUsuarios->horizontalHeader()->setStretchLastSection(true);
    tablaUsuarios->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaUsuarios->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaUsuarios->setAlternatingRowColors(true);
    layout->addWidget(tablaUsuarios);

    QHBoxLayout *rolRow = new QHBoxLayout();
    QComboBox *cmbNuevoRol = new QComboBox();
    cmbNuevoRol->addItems({"admin", "user", "repositor"});
    QPushButton *btnCambiarRol = new QPushButton("Aplicar Rol");
    btnCambiarRol->setCursor(Qt::PointingHandCursor);
    connect(btnCambiarRol, &QPushButton::clicked, this, [this, cmbNuevoRol]() {
        int row = tablaUsuarios->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Error", "Seleccione un usuario");
            return;
        }
        int id = tablaUsuarios->item(row, 0)->text().toInt();
        QString userName = tablaUsuarios->item(row, 1)->text();
        QString newRole = cmbNuevoRol->currentText();

        if (id == m_userId) {
            QMessageBox::warning(this, "Operacion no permitida",
                                 "No podes cambiarte el rol a vos mismo.");
            return;
        }

        if (QMessageBox::question(
                this, "Confirmar",
                QString("Cambiar rol de '%1' a '%2'?").arg(userName, newRole),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            AuthController auth;
            auto result = auth.changeRole(id, newRole);
            if (result.success)
                loadUsers();
            else
                QMessageBox::warning(this, "Error", result.errorMessage);
        }
    });
    rolRow->addWidget(cmbNuevoRol);
    rolRow->addWidget(btnCambiarRol);
    rolRow->addStretch();
    layout->addLayout(rolRow);

    QHBoxLayout *crudRow = new QHBoxLayout();
    QPushButton *btnCrear = new QPushButton("Crear Usuario");
    btnCrear->setObjectName("btnPrimary");
    btnCrear->setCursor(Qt::PointingHandCursor);
    QPushButton *btnEliminar = new QPushButton("Eliminar Usuario");
    btnEliminar->setObjectName("btnDanger");
    btnEliminar->setCursor(Qt::PointingHandCursor);
    btnEliminar->setToolTip("Elimina el usuario seleccionado de la lista");
    connect(btnCrear, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString username = QInputDialog::getText(
            this, "Nuevo Usuario", "Nombre de usuario:",
            QLineEdit::Normal, "", &ok);
        if (!ok || username.trimmed().isEmpty()) return;

        QString password = QInputDialog::getText(
            this, "Nuevo Usuario", "Contrase\u00f1a:",
            QLineEdit::Password, "", &ok);
        if (!ok || password.isEmpty()) return;

        QString confirm = QInputDialog::getText(
            this, "Nuevo Usuario", "Confirmar contrase\u00f1a:",
            QLineEdit::Password, "", &ok);
        if (!ok) return;

        QStringList roles = {"user", "admin", "repositor"};
        QString role = QInputDialog::getItem(
            this, "Nuevo Usuario", "Rol:", roles, 0, false, &ok);
        if (!ok) return;

        AuthController auth;
        auto result = auth.registerUser(username, password, confirm, role);
        if (result.success) {
            QMessageBox::information(this, "Exito",
                                     "Usuario creado correctamente");
            loadUsers();
        } else {
            QMessageBox::warning(this, "Error", result.errorMessage);
        }
    });
    connect(btnEliminar, &QPushButton::clicked, this, [this]() {
        int row = tablaUsuarios->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Error", "Seleccione un usuario");
            return;
        }
        int id = tablaUsuarios->item(row, 0)->text().toInt();
        QString userName = tablaUsuarios->item(row, 1)->text();

        if (id == m_userId) {
            QMessageBox::warning(this, "Operacion no permitida",
                                 "No podes eliminarte a vos mismo.");
            return;
        }

        if (QMessageBox::question(
                this, "Confirmar",
                QString("Eliminar usuario '%1'?").arg(userName),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            AuthController auth;
            auto result = auth.deleteUser(id);
            if (result.success)
                loadUsers();
            else
                QMessageBox::warning(this, "Error", result.errorMessage);
        }
    });
    crudRow->addWidget(btnCrear);
    crudRow->addWidget(btnEliminar);
    crudRow->addStretch();
    layout->addLayout(crudRow);

    loadUsers();
}

void AdminWindow::setupProductsTab(QVBoxLayout *layout) {
    tablaProductos = new QTableWidget(0, 6, this);
    tablaProductos->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Descripcion", "Precio", "Stock", "Stock Minimo"});
    tablaProductos->setAccessibleName("Lista de productos");
    tablaProductos->horizontalHeader()->setStretchLastSection(true);
    tablaProductos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaProductos->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaProductos->setAlternatingRowColors(true);
    layout->addWidget(tablaProductos);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Agregar Producto");
    btnAdd->setObjectName("btnPrimary");
    btnAdd->setCursor(Qt::PointingHandCursor);
    QPushButton *btnEdit = new QPushButton("Editar Producto");
    btnEdit->setCursor(Qt::PointingHandCursor);
    QPushButton *btnDel = new QPushButton("Eliminar Producto");
    btnDel->setObjectName("btnDanger");
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setToolTip("Elimina el producto seleccionado");
    connect(btnAdd, &QPushButton::clicked, this,
            &AdminWindow::showAddProductDialog);
    connect(btnEdit, &QPushButton::clicked, this,
            &AdminWindow::showEditProductDialog);
    connect(btnDel, &QPushButton::clicked, this, &AdminWindow::deleteProduct);
    connect(tablaProductos, &QTableWidget::cellDoubleClicked, this,
            [this](int, int) { showEditProductDialog(); });
    btnRow->addWidget(btnAdd);
    btnRow->addWidget(btnEdit);
    btnRow->addWidget(btnDel);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    loadProducts();
}

void AdminWindow::setupStockReportTab(QVBoxLayout *layout) {
    QLabel *lbl = new QLabel("Productos con stock por debajo del m\u00ednimo:");
    lbl->setStyleSheet("font-size: 14px; font-weight: 500; color: #1d1d1f;");
    layout->addWidget(lbl);

    tablaStockReport = new QTableWidget(0, 5, this);
    tablaStockReport->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Stock Actual", "Stock Minimo", "Faltante"});
    tablaStockReport->setAccessibleName("Reporte de stock bajo");
    tablaStockReport->horizontalHeader()->setStretchLastSection(true);
    tablaStockReport->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaStockReport->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaStockReport->setAlternatingRowColors(true);
    layout->addWidget(tablaStockReport);

    QPushButton *btnRefresh = new QPushButton("Actualizar Reporte");
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this,
            &AdminWindow::loadStockReport);
    layout->addWidget(btnRefresh);

    loadStockReport();
}

void AdminWindow::loadInvoices() {
    tablaFacturas->setRowCount(0);
    auto invoices = m_billing->allInvoices();
    for (const auto &inv : invoices) {
        int row = tablaFacturas->rowCount();
        tablaFacturas->insertRow(row);
        tablaFacturas->setItem(
            row, 0, new QTableWidgetItem(QString::number(inv.id)));
        tablaFacturas->setItem(row, 1,
                               new QTableWidgetItem(inv.clientName));
        tablaFacturas->setItem(row, 2, new QTableWidgetItem(inv.userName));
        tablaFacturas->setItem(row, 3, new QTableWidgetItem(inv.date));
        tablaFacturas->setItem(
            row, 4,
            new QTableWidgetItem(
                QString("$%1").arg(inv.total, 0, 'f', 2)));
        tablaFacturas->setItem(
            row, 5,
            new QTableWidgetItem(QString::number(inv.items.size())));
    }
}

void AdminWindow::loadClients() {
    tablaClientes->setRowCount(0);
    auto clients = m_clients->allClients();
    for (const auto &c : clients) {
        int row = tablaClientes->rowCount();
        tablaClientes->insertRow(row);
        tablaClientes->setItem(
            row, 0, new QTableWidgetItem(QString::number(c.id)));
        tablaClientes->setItem(row, 1, new QTableWidgetItem(c.name));
        tablaClientes->setItem(row, 2, new QTableWidgetItem(c.address));
        tablaClientes->setItem(row, 3, new QTableWidgetItem(c.phone));
    }
}

void AdminWindow::loadUsers() {
    tablaUsuarios->setRowCount(0);
    AuthController auth;
    auto users = auth.allUsers();
    for (const auto &u : users) {
        int row = tablaUsuarios->rowCount();
        tablaUsuarios->insertRow(row);
        tablaUsuarios->setItem(
            row, 0, new QTableWidgetItem(QString::number(u.id)));
        tablaUsuarios->setItem(row, 1, new QTableWidgetItem(u.username));
        tablaUsuarios->setItem(row, 2, new QTableWidgetItem(u.role));
    }
}

void AdminWindow::loadProducts() {
    tablaProductos->setRowCount(0);
    auto products = m_inventory->allProducts();
    for (const auto &p : products) {
        int row = tablaProductos->rowCount();
        tablaProductos->insertRow(row);
        tablaProductos->setItem(
            row, 0, new QTableWidgetItem(QString::number(p.id)));
        tablaProductos->setItem(row, 1, new QTableWidgetItem(p.name));
        tablaProductos->setItem(row, 2,
                                new QTableWidgetItem(p.description));
        tablaProductos->setItem(
            row, 3,
            new QTableWidgetItem(
                QString("$%1").arg(p.price, 0, 'f', 2)));
        tablaProductos->setItem(
            row, 4, new QTableWidgetItem(QString::number(p.stock)));
        tablaProductos->setItem(
            row, 5, new QTableWidgetItem(QString::number(p.minStock)));
    }
}

void AdminWindow::loadStockReport() {
    tablaStockReport->setRowCount(0);
    auto low = m_inventory->lowStockReport();
    for (const auto &p : low) {
        int row = tablaStockReport->rowCount();
        tablaStockReport->insertRow(row);
        tablaStockReport->setItem(
            row, 0, new QTableWidgetItem(QString::number(p.id)));
        tablaStockReport->setItem(row, 1, new QTableWidgetItem(p.name));
        tablaStockReport->setItem(
            row, 2, new QTableWidgetItem(QString::number(p.stock)));
        tablaStockReport->setItem(
            row, 3, new QTableWidgetItem(QString::number(p.minStock)));
        tablaStockReport->setItem(
            row, 4,
            new QTableWidgetItem(QString::number(p.minStock - p.stock)));
    }
}

void AdminWindow::showNewInvoiceDialog() {
    InvoiceDialog dialog(*m_billing, m_userId, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadInvoices();
        loadProducts();
    }
}

void AdminWindow::showAddProductDialog() {
    bool ok;
    QString name = QInputDialog::getText(this, "Nuevo Producto", "Nombre:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty())
        return;

    QString desc = QInputDialog::getText(this, "Nuevo Producto", "Descripcion:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok)
        return;

    double price = QInputDialog::getDouble(this, "Nuevo Producto", "Precio:",
                                           0, 0, 999999, 2, &ok);
    if (!ok)
        return;

    int stock = QInputDialog::getInt(this, "Nuevo Producto", "Stock:", 0, 0,
                                     999999, 1, &ok);
    if (!ok)
        return;

    int minStock = QInputDialog::getInt(this, "Nuevo Producto",
                                        "Stock Minimo:", 0, 0, 999999, 1, &ok);
    if (!ok)
        return;

    auto result =
        m_inventory->addProduct(name, desc, price, stock, minStock);
    if (result.success)
        loadProducts();
    else
        QMessageBox::warning(this, "Error", result.errorMessage);
}

void AdminWindow::showEditProductDialog() {
    int row = tablaProductos->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un producto");
        return;
    }

    auto prodResult = m_inventory->findProduct(
        tablaProductos->item(row, 0)->text().toInt());
    if (!prodResult.success) {
        QMessageBox::warning(this, "Error", prodResult.message);
        return;
    }

    Product p = prodResult.value;
    bool ok;

    QString name = QInputDialog::getText(this, "Editar Producto", "Nombre:",
                                         QLineEdit::Normal, p.name, &ok);
    if (!ok || name.trimmed().isEmpty())
        return;

    QString desc = QInputDialog::getText(this, "Editar Producto",
                                         "Descripcion:", QLineEdit::Normal,
                                         p.description, &ok);
    if (!ok)
        return;

    double price = QInputDialog::getDouble(this, "Editar Producto", "Precio:",
                                           p.price, 0, 999999, 2, &ok);
    if (!ok)
        return;

    int stock = QInputDialog::getInt(this, "Editar Producto", "Stock:",
                                     p.stock, 0, 999999, 1, &ok);
    if (!ok)
        return;

    int minStock = QInputDialog::getInt(this, "Editar Producto",
                                        "Stock Minimo:", p.minStock, 0, 999999,
                                        1, &ok);
    if (!ok)
        return;

    p.name = name.trimmed();
    p.description = desc.trimmed();
    p.price = price;
    p.stock = stock;
    p.minStock = minStock;

    auto result = m_inventory->updateProduct(p);
    if (result.success)
        loadProducts();
    else
        QMessageBox::warning(this, "Error", result.errorMessage);
}

void AdminWindow::deleteProduct() {
    int row = tablaProductos->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un producto");
        return;
    }

    int id = tablaProductos->item(row, 0)->text().toInt();
    QString name = tablaProductos->item(row, 1)->text();

    if (QMessageBox::question(this, "Confirmar",
                              "Eliminar producto '" + name + "'?",
                              QMessageBox::Yes | QMessageBox::No) ==
        QMessageBox::Yes) {
        auto result = m_inventory->removeProduct(id);
        if (result.success)
            loadProducts();
        else
            QMessageBox::warning(this, "Error", result.errorMessage);
    }
}

void AdminWindow::showAddClientDialog() {
    bool ok;
    QString name = QInputDialog::getText(this, "Nuevo Cliente", "Nombre:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty())
        return;

    QString address = QInputDialog::getText(this, "Nuevo Cliente",
                                            "Direccion:", QLineEdit::Normal,
                                            "", &ok);
    if (!ok)
        return;

    QString phone = QInputDialog::getText(this, "Nuevo Cliente", "Telefono:",
                                          QLineEdit::Normal, "", &ok);
    if (!ok)
        return;

    auto result = m_clients->addClient(name, address, phone);
    if (result.success)
        loadClients();
    else
        QMessageBox::critical(this, "Error", result.errorMessage);
}

void AdminWindow::showEditClientDialog() {
    int row = tablaClientes->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un cliente");
        return;
    }

    int id = tablaClientes->item(row, 0)->text().toInt();
    auto result = m_clients->findClient(id);
    if (!result.success) {
        QMessageBox::warning(this, "Error", result.message);
        return;
    }

    Client c = result.value;
    bool ok;

    QString name = QInputDialog::getText(this, "Editar Cliente", "Nombre:",
                                         QLineEdit::Normal, c.name, &ok);
    if (!ok || name.trimmed().isEmpty())
        return;

    QString address = QInputDialog::getText(this, "Editar Cliente",
                                            "Direccion:", QLineEdit::Normal,
                                            c.address, &ok);
    if (!ok)
        return;

    QString phone = QInputDialog::getText(this, "Editar Cliente", "Telefono:",
                                          QLineEdit::Normal, c.phone, &ok);
    if (!ok)
        return;

    c.name = name.trimmed();
    c.address = address.trimmed();
    c.phone = phone.trimmed();
    auto updResult = m_clients->updateClient(c);
    if (updResult.success)
        loadClients();
    else
        QMessageBox::critical(this, "Error", updResult.errorMessage);
}

void AdminWindow::deleteClient() {
    int row = tablaClientes->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Seleccione un cliente");
        return;
    }

    int id = tablaClientes->item(row, 0)->text().toInt();
    QString name = tablaClientes->item(row, 1)->text();

    if (QMessageBox::question(this, "Confirmar",
                              "Eliminar cliente '" + name + "'?",
                              QMessageBox::Yes | QMessageBox::No) ==
        QMessageBox::Yes) {
        auto result = m_clients->removeClient(id);
        if (result.success)
            loadClients();
        else
            QMessageBox::warning(this, "Error", result.errorMessage);
    }
}
