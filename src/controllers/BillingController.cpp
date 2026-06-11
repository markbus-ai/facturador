#include "BillingController.h"
#include "core/Validation.h"
#include "database/InvoiceRepository.h"
#include "database/ClientRepository.h"
#include "database/ProductRepository.h"

BillingController::BillingController(const IAccessContract &access, int userId)
    : m_access(access), m_userId(userId), m_invoiceService(access) {}

QList<Client> BillingController::allClients() {
    return ClientRepository::instance().all();
}

QList<Product> BillingController::allProducts() {
    return ProductRepository::instance().all();
}

QList<Invoice> BillingController::allInvoices() {
    if (m_access.canViewAllInvoices())
        return InvoiceRepository::instance().all();
    return myInvoices();
}

QList<Invoice> BillingController::myInvoices() {
    if (!m_access.canCreateInvoice())
        return {};
    return InvoiceRepository::instance().byUser(m_userId);
}

ResultOr<Invoice> BillingController::findInvoice(int id) {
    if (!Validation::isIdValid(id))
        return {false, "ID de factura inv\u00e1lido", Invoice{}};
    return InvoiceRepository::instance().findById(id);
}

Result BillingController::submitInvoice(int clientId, int userId,
                                         const QList<InvoiceItem> &items,
                                         std::unique_ptr<IDiscountStrategy> discount) {
    return m_invoiceService.submitInvoice(clientId, userId, items, std::move(discount));
}

Result BillingController::deleteInvoice(int invoiceId) {
    if (!m_access.canViewAllInvoices())
        return Result::fail("No tiene permisos para eliminar facturas");

    if (!Validation::isIdValid(invoiceId))
        return Result::fail("ID de factura inv\u00e1lido");

    return InvoiceRepository::instance().remove(invoiceId);
}
