#pragma once
#include "core/Invoice.h"
#include "core/Product.h"
#include "core/Client.h"
#include "core/Result.h"
#include "core/IAccessContract.h"
#include "core/IDiscountStrategy.h"
#include "services/InvoiceService.h"
#include <QList>
#include <memory>

class BillingController {
public:
    explicit BillingController(const IAccessContract &access, int userId);

    QList<Client> allClients();
    QList<Product> allProducts();
    QList<Invoice> allInvoices();
    QList<Invoice> myInvoices();
    ResultOr<Invoice> findInvoice(int id);
    Result submitInvoice(int clientId, int userId,
                         const QList<InvoiceItem> &items,
                         std::unique_ptr<IDiscountStrategy> discount);
    Result deleteInvoice(int invoiceId);

private:
    const IAccessContract &m_access;
    int m_userId;
    InvoiceService m_invoiceService;
};
