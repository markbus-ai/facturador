#pragma once
#include <QList>
#include "core/Invoice.h"
#include "core/Result.h"

class QSqlQuery;

class InvoiceRepository {
public:
    static InvoiceRepository &instance();

    QList<Invoice> all();
    QList<Invoice> byUser(int userId);
    ResultOr<Invoice> findById(int id);
    Result save(Invoice &invoice);
    Result remove(int id);

private:
    InvoiceRepository() = default;
    Invoice rowToInvoice(const QSqlQuery &q) const;
    InvoiceItem rowToItem(const QSqlQuery &q) const;
};
