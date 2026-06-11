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
#include "core/IAccessContract.h"
#include "controllers/InventoryController.h"
#include "controllers/BillingController.h"
#include "controllers/ClientController.h"
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

    QTabWidget *tabs;
    QTableWidget *tablaFacturas;
    QTableWidget *tablaClientes;
    QTableWidget *tablaUsuarios;
    QTableWidget *tablaProductos;
    QTableWidget *tablaStockReport;

    void setupInvoicesTab(QVBoxLayout *layout);
    void setupClientsTab(QVBoxLayout *layout);
    void setupUsersTab(QVBoxLayout *layout);
    void setupProductsTab(QVBoxLayout *layout);
    void setupStockReportTab(QVBoxLayout *layout);

    void loadInvoices();
    void loadClients();
    void loadUsers();
    void loadProducts();
    void loadStockReport();

    void showNewInvoiceDialog();
    void showAddProductDialog();
    void showEditProductDialog();
    void deleteProduct();
    void showAddClientDialog();
    void showEditClientDialog();
    void deleteClient();
};
