#include "ProductRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
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
    return p;
}

QList<Product> ProductRepository::all() {
    QList<Product> list;
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.exec("SELECT * FROM products ORDER BY name");
    while (q.next())
        list.append(rowToProduct(q));
    return list;
}

ResultOr<Product> ProductRepository::findById(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT * FROM products WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next())
        return {true, {}, rowToProduct(q)};
    return {false, "Producto no encontrado", Product{}};
}

Result ProductRepository::save(const Product &product) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("INSERT INTO products (name, description, price, stock, min_stock) "
              "VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(product.name);
    q.addBindValue(product.description);
    q.addBindValue(product.price);
    q.addBindValue(product.stock);
    q.addBindValue(product.minStock);
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
              "stock=?, min_stock=? WHERE id=?");
    q.addBindValue(product.name);
    q.addBindValue(product.description);
    q.addBindValue(product.price);
    q.addBindValue(product.stock);
    q.addBindValue(product.minStock);
    q.addBindValue(product.id);
    if (!q.exec()) {
        qDebug() << "Error actualizando producto:" << q.lastError().text();
        return Result::fail("Error al actualizar producto");
    }
    return Result::ok();
}

Result ProductRepository::remove(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("DELETE FROM products WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qDebug() << "Error eliminando producto:" << q.lastError().text();
        return Result::fail("No se puede eliminar: el producto tiene facturas asociadas");
    }
    return Result::ok();
}

QList<Product> ProductRepository::lowStock() {
    QList<Product> list;
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.exec("SELECT * FROM products WHERE stock < min_stock ORDER BY name");
    while (q.next())
        list.append(rowToProduct(q));
    return list;
}
