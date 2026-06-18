#include "InvoiceService.h"
#include "InvoiceCalculator.h"
#include "core/Validation.h"
#include "database/ProductRepository.h"
#include "database/InvoiceRepository.h"
#include "database/DatabaseManager.h"
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtGlobal>

InvoiceService::InvoiceService(const IAccessContract &access)
    : m_access(access) {}

Result InvoiceService::submitInvoice(int clientId, int userId,
                                     const QList<InvoiceItem> &items,
                                     std::unique_ptr<IDiscountStrategy> discount) {
    if (!m_access.canCreateInvoice())
        return Result::fail("No tiene permisos para crear facturas");

    if (!Validation::isIdValid(clientId))
        return Result::fail("Seleccione un cliente v\u00e1lido");

    if (!Validation::isIdValid(userId))
        return Result::fail("Error de sesi\u00f3n: usuario inv\u00e1lido");

    if (items.isEmpty())
        return Result::fail("La factura debe tener al menos un item");

    if (!discount)
        discount = std::make_unique<NoDiscount>();

    Result itemsResult = validateItems(items);
    if (!itemsResult.success) return itemsResult;

    InvoiceTotals totals = InvoiceCalculator::calculateBoth(items, *discount);

    if (totals.total > 99999999.99)
        return Result::fail("El total de la factura supera el l\u00edmite permitido");

    auto &db = DatabaseManager::instance().database();
    db.transaction();

    Result stockResult = deductStock(items);
    if (!stockResult.success) {
        db.rollback();
        return stockResult;
    }

    Invoice invoice;
    invoice.clientId = clientId;
    invoice.userId = userId;
    invoice.date = QDate::currentDate().toString("yyyy-MM-dd");
    invoice.subtotal = totals.subtotal;
    invoice.discountAmount = totals.discountAmount;
    invoice.discountStrategyName = discount->name();
    invoice.vat = totals.vat;
    invoice.total = totals.total;
    invoice.items = items;

    auto saveResult = InvoiceRepository::instance().save(invoice);
    if (!saveResult.success) {
        db.rollback();
        return saveResult;
    }

    if (!db.commit()) {
        db.rollback();
        return Result::fail("Error al confirmar la transaccion");
    }

    return Result::ok();
}

Result InvoiceService::validateItems(const QList<InvoiceItem> &items) const {
    for (const auto &item : items) {
        if (!Validation::isIdValid(item.productId))
            return Result::fail("ID de producto inv\u00e1lido en uno de los items");

        if (!Validation::isNotEmpty(item.productName))
            return Result::fail("Todos los items deben tener descripci\u00f3n");

        if (item.quantity <= 0)
            return Result::fail("La cantidad debe ser mayor a cero");

        if (item.quantity > 999999)
            return Result::fail("La cantidad no puede superar 999999");

        if (!Validation::isPositive(item.unitPrice))
            return Result::fail("El precio unitario debe ser mayor a cero");

        if (item.discountType == "percentage" &&
            !Validation::isInRange(item.discountValue, 0, 100))
            return Result::fail("El descuento porcentual debe estar entre 0 y 100");

        if (item.discountType == "nominal" && item.discountValue < 0)
            return Result::fail("El descuento fijo no puede ser negativo");

        auto prodResult = ProductRepository::instance().findById(item.productId);
        if (!prodResult.success)
            return Result::fail("Producto no encontrado: " + item.productName);
    }
    return Result::ok();
}

Result InvoiceService::deductStock(const QList<InvoiceItem> &items) {
    auto &db = DatabaseManager::instance().database();
    for (const auto &item : items) {
        QSqlQuery lockQ(db);
        lockQ.prepare("SELECT stock FROM products WHERE id = ? FOR UPDATE");
        lockQ.addBindValue(item.productId);
        if (!lockQ.exec() || !lockQ.next())
            return Result::fail("Producto no encontrado: " + item.productName);

        QSqlQuery q(db);
        q.prepare("UPDATE products SET stock = stock - ? WHERE id = ? AND stock >= ?");
        q.addBindValue(item.quantity);
        q.addBindValue(item.productId);
        q.addBindValue(item.quantity);
        if (!q.exec()) {
            qDebug() << "Error deduciendo stock:" << q.lastError().text();
            return Result::fail("Error al actualizar stock");
        }
        if (q.numRowsAffected() == 0) {
            return Result::fail("Stock insuficiente para " + item.productName +
                                " (solicitado: " + QString::number(item.quantity) + ")");
        }
    }
    return Result::ok();
}
