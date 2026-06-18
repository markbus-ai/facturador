#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <memory>
#include "core/IAccessContract.h"
#include "controllers/InventoryController.h"
#include "controllers/SupplierController.h"

class RepositorWindow : public QMainWindow {
    Q_OBJECT

public:
    RepositorWindow(const QString &username, int userId,
                    QWidget *parent = nullptr);
    ~RepositorWindow() = default;

signals:
    void loggedOut();

private:
    QString m_username;
    int m_userId;
    std::unique_ptr<IAccessContract> m_access;
    std::unique_ptr<InventoryController> m_inventory;
    std::unique_ptr<SupplierController> m_suppliers;

    QTabWidget *tabs;
    QTableWidget *tablaProductos;
    QTableWidget *tablaStockBajo;
    QTableWidget *tablaProveedores;

    void setupProductsTab(QVBoxLayout *layout);
    void setupSuppliersTab(QVBoxLayout *layout);

    void loadProducts();
    void loadStockReport();
    void loadSuppliers();
    void updateStock();
    void showAddSupplierDialog();
    void showEditSupplierDialog();
    void deleteSupplier();
};
