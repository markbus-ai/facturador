#pragma once
#include <QList>
#include "core/Supplier.h"
#include "core/Result.h"

class QSqlQuery;

class SupplierRepository {
public:
    static SupplierRepository &instance();

    QList<Supplier> all();
    ResultOr<Supplier> findById(int id);
    Result save(const Supplier &supplier);
    Result update(const Supplier &supplier);
    Result remove(int id);

private:
    SupplierRepository() = default;
    Supplier rowToSupplier(const QSqlQuery &q) const;
};
