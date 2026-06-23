#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include "core/IAccessContract.h"
#include "controllers/BillingController.h"
#include "controllers/InventoryController.h"

class UserWindow : public QMainWindow {
    Q_OBJECT

public:
    UserWindow(const QString &username, int userId,
               QWidget *parent = nullptr);
    ~UserWindow() = default;

signals:
    void loggedOut();

private:
    QString m_username;
    int m_userId;
    std::unique_ptr<IAccessContract> m_access;
    std::unique_ptr<BillingController> m_billing;
    std::unique_ptr<InventoryController> m_inventory;

    QTableWidget *tablaFacturas;
    QTableWidget *tablaProductos;
    QPushButton *btnNuevaFactura;

    void loadInvoices();
    void loadProducts();
    void showNewInvoiceDialog();
    void showInvoiceDetail();
};
