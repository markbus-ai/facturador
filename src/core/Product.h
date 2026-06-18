#pragma once
#include <QString>

struct Product {
    int id = 0;
    QString name;
    QString description;
    double price = 0.0;
    int stock = 0;
    int minStock = 0;
    int supplierId = 0;
    QString supplierName;

    bool needsRestock() const { return stock < minStock; }
};
