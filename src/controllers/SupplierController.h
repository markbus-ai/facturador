#pragma once
#include "core/Supplier.h"
#include "core/Result.h"
#include "core/IAccessContract.h"
#include <QList>

class SupplierController {
public:
    explicit SupplierController(const IAccessContract &access);

    QList<Supplier> allSuppliers();
    ResultOr<Supplier> findSupplier(int id);
    Result addSupplier(const QString &name, const QString &contact,
                       const QString &phone, const QString &email,
                       const QString &address, const QString &cuit);
    Result updateSupplier(const Supplier &supplier);
    Result removeSupplier(int id);

private:
    const IAccessContract &m_access;
};
