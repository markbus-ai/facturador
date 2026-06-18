#include "SupplierRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SupplierRepository &SupplierRepository::instance() {
    static SupplierRepository repo;
    return repo;
}

Supplier SupplierRepository::rowToSupplier(const QSqlQuery &q) const {
    Supplier s;
    s.id = q.value("id").toInt();
    s.name = q.value("name").toString();
    s.contact = q.value("contact").toString();
    s.phone = q.value("phone").toString();
    s.email = q.value("email").toString();
    s.address = q.value("address").toString();
    s.cuit = q.value("cuit").toString();
    return s;
}

QList<Supplier> SupplierRepository::all() {
    QList<Supplier> list;
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.exec("SELECT * FROM suppliers ORDER BY name");
    while (q.next())
        list.append(rowToSupplier(q));
    return list;
}

ResultOr<Supplier> SupplierRepository::findById(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT * FROM suppliers WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next())
        return {true, {}, rowToSupplier(q)};
    return {false, "Proveedor no encontrado", Supplier{}};
}

Result SupplierRepository::save(const Supplier &supplier) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("INSERT INTO suppliers (name, contact, phone, email, address, cuit) "
              "VALUES (?, ?, ?, ?, ?, ?)");
    q.addBindValue(supplier.name);
    q.addBindValue(supplier.contact);
    q.addBindValue(supplier.phone);
    q.addBindValue(supplier.email);
    q.addBindValue(supplier.address);
    q.addBindValue(supplier.cuit);
    if (!q.exec()) {
        qDebug() << "Error guardando proveedor:" << q.lastError().text();
        return Result::fail("Error al guardar proveedor");
    }
    return Result::ok();
}

Result SupplierRepository::update(const Supplier &supplier) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("UPDATE suppliers SET name=?, contact=?, phone=?, email=?, "
              "address=?, cuit=? WHERE id=?");
    q.addBindValue(supplier.name);
    q.addBindValue(supplier.contact);
    q.addBindValue(supplier.phone);
    q.addBindValue(supplier.email);
    q.addBindValue(supplier.address);
    q.addBindValue(supplier.cuit);
    q.addBindValue(supplier.id);
    if (!q.exec()) {
        qDebug() << "Error actualizando proveedor:" << q.lastError().text();
        return Result::fail("Error al actualizar proveedor");
    }
    if (q.numRowsAffected() == 0)
        return Result::fail("Proveedor no encontrado");
    return Result::ok();
}

Result SupplierRepository::remove(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("DELETE FROM suppliers WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        if (q.lastError().text().contains("foreign key")) {
            qDebug() << "Proveedor tiene productos:" << q.lastError().text();
            return Result::fail("No se puede eliminar: el proveedor tiene productos asociados");
        }
        qDebug() << "Error eliminando proveedor:" << q.lastError().text();
        return Result::fail("Error al eliminar proveedor");
    }
    if (q.numRowsAffected() == 0)
        return Result::fail("Proveedor no encontrado");
    return Result::ok();
}
