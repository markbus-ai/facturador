#pragma once
#include "core/Invoice.h"
#include "core/Result.h"
#include "core/IDiscountStrategy.h"
#include "core/IAccessContract.h"
#include <QList>
#include <memory>

class InvoiceService {
public:
    explicit InvoiceService(const IAccessContract &access);

    Result submitInvoice(int clientId, int userId,
                         const QList<InvoiceItem> &items,
                         std::unique_ptr<IDiscountStrategy> discount);

private:
    const IAccessContract &m_access;

    Result validateItems(const QList<InvoiceItem> &items) const;
    Result deductStock(const QList<InvoiceItem> &items);
};
