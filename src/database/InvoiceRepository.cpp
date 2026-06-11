#include "InvoiceRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>

InvoiceRepository &InvoiceRepository::instance() {
    static InvoiceRepository repo;
    return repo;
}

Invoice InvoiceRepository::rowToInvoice(const QSqlQuery &q) const {
    Invoice inv;
    inv.id = q.value("id").toInt();
    inv.clientId = q.value("client_id").toInt();
    inv.clientName = q.value("client_name").toString();
    inv.userId = q.value("user_id").toInt();
    inv.userName = q.value("username").toString();
    inv.date = q.value("date").toString();
    inv.subtotal = q.value("subtotal").toDouble();
    inv.discountAmount = q.value("discount_amount").toDouble();
    inv.discountStrategyName = q.value("discount_strategy").toString();
    inv.vat = q.value("vat").toDouble();
    inv.total = q.value("total").toDouble();
    return inv;
}

InvoiceItem InvoiceRepository::rowToItem(const QSqlQuery &q) const {
    InvoiceItem item;
    item.productId = q.value("product_id").toInt();
    item.productName = q.value("product_name").toString();
    item.quantity = q.value("quantity").toInt();
    item.unitPrice = q.value("unit_price").toDouble();
    item.subtotal = q.value("subtotal").toDouble();
    item.discountType = q.value("discount_type").toString();
    item.discountValue = q.value("discount_value").toDouble();
    return item;
}

QList<Invoice> InvoiceRepository::all() {
    QList<Invoice> list;
    auto &db = DatabaseManager::instance().database();

    QSqlQuery q(db);
    q.exec("SELECT i.*, c.name AS client_name, u.username "
           "FROM invoices i "
           "JOIN clients c ON c.id = i.client_id "
           "JOIN users u ON u.id = i.user_id "
           "ORDER BY i.id DESC");

    QList<int> invoiceIds;
    QMap<int, int> idToIndex;
    while (q.next()) {
        Invoice inv = rowToInvoice(q);
        int idx = list.size();
        idToIndex[inv.id] = idx;
        invoiceIds.append(inv.id);
        list.append(inv);
    }

    if (!invoiceIds.isEmpty()) {
        QStringList placeholders(invoiceIds.size(), "?");
        QSqlQuery itemQ(db);
        itemQ.prepare("SELECT * FROM invoice_items WHERE invoice_id IN (" +
                      placeholders.join(",") + ")");
        for (int id : invoiceIds)
            itemQ.addBindValue(id);
        itemQ.exec();
        while (itemQ.next()) {
            int invoiceId = itemQ.value("invoice_id").toInt();
            list[idToIndex[invoiceId]].items.append(rowToItem(itemQ));
        }
    }

    return list;
}

QList<Invoice> InvoiceRepository::byUser(int userId) {
    QList<Invoice> list;
    auto &db = DatabaseManager::instance().database();

    QSqlQuery q(db);
    q.prepare("SELECT i.*, c.name AS client_name, u.username "
              "FROM invoices i "
              "JOIN clients c ON c.id = i.client_id "
              "JOIN users u ON u.id = i.user_id "
              "WHERE i.user_id = ? "
              "ORDER BY i.id DESC");
    q.addBindValue(userId);

    if (!q.exec())
        return list;

    QList<int> invoiceIds;
    QMap<int, int> idToIndex;
    while (q.next()) {
        Invoice inv = rowToInvoice(q);
        int idx = list.size();
        idToIndex[inv.id] = idx;
        invoiceIds.append(inv.id);
        list.append(inv);
    }

    if (!invoiceIds.isEmpty()) {
        QStringList placeholders(invoiceIds.size(), "?");
        QSqlQuery itemQ(db);
        itemQ.prepare("SELECT * FROM invoice_items WHERE invoice_id IN (" +
                      placeholders.join(",") + ")");
        for (int id : invoiceIds)
            itemQ.addBindValue(id);
        itemQ.exec();
        while (itemQ.next()) {
            int invoiceId = itemQ.value("invoice_id").toInt();
            list[idToIndex[invoiceId]].items.append(rowToItem(itemQ));
        }
    }

    return list;
}

ResultOr<Invoice> InvoiceRepository::findById(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT i.*, c.name AS client_name, u.username "
              "FROM invoices i "
              "JOIN clients c ON c.id = i.client_id "
              "JOIN users u ON u.id = i.user_id "
              "WHERE i.id = ?");
    q.addBindValue(id);

    if (!q.exec() || !q.next())
        return {false, "Factura no encontrada", Invoice{}};

    Invoice inv = rowToInvoice(q);

    QSqlQuery itemQ(db);
    itemQ.prepare("SELECT * FROM invoice_items WHERE invoice_id = ?");
    itemQ.addBindValue(inv.id);
    itemQ.exec();
    while (itemQ.next())
        inv.items.append(rowToItem(itemQ));

    return {true, {}, inv};
}

Result InvoiceRepository::save(Invoice &invoice) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);

    q.prepare("INSERT INTO invoices (client_id, user_id, date, subtotal, "
              "discount_amount, discount_strategy, vat, total) "
              "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    q.addBindValue(invoice.clientId);
    q.addBindValue(invoice.userId);
    q.addBindValue(invoice.date);
    q.addBindValue(invoice.subtotal);
    q.addBindValue(invoice.discountAmount);
    q.addBindValue(invoice.discountStrategyName);
    q.addBindValue(invoice.vat);
    q.addBindValue(invoice.total);

    if (!q.exec()) {
        qDebug() << "Error guardando factura:" << q.lastError().text();
        return Result::fail("Error al guardar factura");
    }

    invoice.id = q.lastInsertId().toInt();

    for (const auto &item : invoice.items) {
        QSqlQuery ins(db);
        ins.prepare("INSERT INTO invoice_items (invoice_id, product_id, product_name, "
                    "quantity, unit_price, subtotal, discount_type, discount_value) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        ins.addBindValue(invoice.id);
        ins.addBindValue(item.productId);
        ins.addBindValue(item.productName);
        ins.addBindValue(item.quantity);
        ins.addBindValue(item.unitPrice);
        ins.addBindValue(item.subtotal);
        ins.addBindValue(item.discountType);
        ins.addBindValue(item.discountValue);
        if (!ins.exec()) {
            qDebug() << "Error guardando item:" << ins.lastError().text();
            return Result::fail("Error al guardar item de factura");
        }
    }

    return Result::ok();
}

Result InvoiceRepository::remove(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);

    q.prepare("DELETE FROM invoices WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qDebug() << "Error eliminando factura:" << q.lastError().text();
        return Result::fail("Error al eliminar la factura");
    }

    return Result::ok();
}
