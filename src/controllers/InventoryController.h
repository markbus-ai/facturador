#pragma once
#include "core/Product.h"
#include "core/Result.h"
#include "core/IAccessContract.h"
#include <QList>

class InventoryController {
public:
    explicit InventoryController(const IAccessContract &access);

    QList<Product> allProducts();
    ResultOr<Product> findProduct(int id);
    Result addProduct(const QString &name, const QString &description,
                      double price, int stock, int minStock, int supplierId = 0);
    Result updateProduct(const Product &product);
    Result removeProduct(int id);
    QList<Product> lowStockReport();

private:
    const IAccessContract &m_access;
};
