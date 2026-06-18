#include "ProductRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

ProductRepository &ProductRepository::instance() {
    static ProductRepository repo;
    return repo;
}

Product ProductRepository::rowToProduct(const QSqlQuery &q) const {
    Product p;
    p.id = q.value("id").toInt();
    p.name = q.value("name").toString();
    p.description = q.value("description").toString();
    p.price = q.value("price").toDouble();
    p.stock = q.value("stock").toInt();
    p.minStock = q.value("min_stock").toInt();
    p.supplierId = q.value("supplier_id").toInt();
    p.supplierName = q.value("supplier_name").toString();
    return p;
}

static const char *PRODUCT_SELECT =
    "SELECT p.*, COALESCE(s.name, '') AS supplier_name "
    "FROM products p "
    "LEFT JOIN suppliers s ON s.id = p.supplier_id ";

QList<Product> ProductRepository::all() {
    QList<Product> list;
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.exec(QString(PRODUCT_SELECT) + "ORDER BY p.name");
    while (q.next())
        list.append(rowToProduct(q));
    return list;
}

ResultOr<Product> ProductRepository::findById(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare(QString(PRODUCT_SELECT) + "WHERE p.id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next())
        return {true, {}, rowToProduct(q)};
    return {false, "Producto no encontrado", Product{}};
}

Result ProductRepository::save(const Product &product) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("INSERT INTO products (name, description, price, stock, min_stock, supplier_id) "
              "VALUES (?, ?, ?, ?, ?, ?)");
    q.addBindValue(product.name);
    q.addBindValue(product.description);
    q.addBindValue(product.price);
    q.addBindValue(product.stock);
    q.addBindValue(product.minStock);
    q.addBindValue(product.supplierId > 0 ? QVariant(product.supplierId) : QVariant());
    if (!q.exec()) {
        qDebug() << "Error guardando producto:" << q.lastError().text();
        return Result::fail("Error al guardar producto");
    }
    return Result::ok();
}

Result ProductRepository::update(const Product &product) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("UPDATE products SET name=?, description=?, price=?, "
              "stock=?, min_stock=?, supplier_id=? WHERE id=?");
    q.addBindValue(product.name);
    q.addBindValue(product.description);
    q.addBindValue(product.price);
    q.addBindValue(product.stock);
    q.addBindValue(product.minStock);
    q.addBindValue(product.supplierId > 0 ? QVariant(product.supplierId) : QVariant());
    q.addBindValue(product.id);
    if (!q.exec()) {
        qDebug() << "Error actualizando producto:" << q.lastError().text();
        return Result::fail("Error al actualizar producto");
    }
    if (q.numRowsAffected() == 0)
        return Result::fail("Producto no encontrado");
    return Result::ok();
}

Result ProductRepository::remove(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("DELETE FROM products WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        if (q.lastError().text().contains("foreign key"))
            return Result::fail("No se puede eliminar: el producto tiene facturas asociadas");
        qDebug() << "Error eliminando producto:" << q.lastError().text();
        return Result::fail("Error al eliminar producto");
    }
    if (q.numRowsAffected() == 0)
        return Result::fail("Producto no encontrado");
    return Result::ok();
}

QList<Product> ProductRepository::lowStock() {
    QList<Product> list;
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.exec(QString(PRODUCT_SELECT) + "WHERE p.stock < p.min_stock ORDER BY p.name");
    while (q.next())
        list.append(rowToProduct(q));
    return list;
}
