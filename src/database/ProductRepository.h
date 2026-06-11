#pragma once
#include <QList>
#include "core/Product.h"
#include "core/Result.h"

class QSqlQuery;

class ProductRepository {
public:
    static ProductRepository &instance();

    QList<Product> all();
    ResultOr<Product> findById(int id);
    Result save(const Product &product);
    Result update(const Product &product);
    Result remove(int id);
    QList<Product> lowStock();

private:
    ProductRepository() = default;
    Product rowToProduct(const QSqlQuery &q) const;
};
