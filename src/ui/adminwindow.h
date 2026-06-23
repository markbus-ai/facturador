#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QLineEdit>
#include "core/IAccessContract.h"
#include "controllers/InventoryController.h"
#include "controllers/BillingController.h"
#include "controllers/ClientController.h"
#include "controllers/SupplierController.h"
#include <memory>

class AdminWindow : public QMainWindow {
    Q_OBJECT

public:
    AdminWindow(const QString &username, int userId,
                QWidget *parent = nullptr);
    ~AdminWindow() = default;

signals:
    void loggedOut();

private:
    QString m_username;
    int m_userId;
    std::unique_ptr<IAccessContract> m_access;
    std::unique_ptr<InventoryController> m_inventory;
    std::unique_ptr<BillingController> m_billing;
    std::unique_ptr<ClientController> m_clients;
    std::unique_ptr<SupplierController> m_suppliers;

    QTabWidget *tabs;
    QTableWidget *tablaFacturas;
    QTableWidget *tablaClientes;
    QTableWidget *tablaUsuarios;
    QTableWidget *tablaProductos;
    QTableWidget *tablaStockReport;
    QTableWidget *tablaProveedores;
    QLineEdit *searchClientes;
    QLineEdit *searchProveedores;

    void setupInvoicesTab(QVBoxLayout *layout);
    void setupClientsTab(QVBoxLayout *layout);
    void setupSuppliersTab(QVBoxLayout *layout);
    void setupUsersTab(QVBoxLayout *layout);
    void setupProductsTab(QVBoxLayout *layout);
    void setupStockReportTab(QVBoxLayout *layout);

    void loadInvoices();
    void loadClients();
    void loadSuppliers();
    void loadUsers();
    void loadProducts();
    void loadStockReport();

    void filterClientes(const QString &text);
    void filterProveedores(const QString &text);
    void showInvoiceDetail();

    void showNewInvoiceDialog();
    void showAddProductDialog();
    void showEditProductDialog();
    void deleteProduct();
    void showAddClientDialog();
    void showEditClientDialog();
    void deleteClient();
    void showAddSupplierDialog();
    void showEditSupplierDialog();
    void deleteSupplier();
};
