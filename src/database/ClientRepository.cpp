#include "ClientRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ClientRepository &ClientRepository::instance() {
    static ClientRepository repo;
    return repo;
}

Client ClientRepository::rowToClient(const QSqlQuery &q) const {
    Client c;
    c.id = q.value("id").toInt();
    c.name = q.value("name").toString();
    c.address = q.value("address").toString();
    c.phone = q.value("phone").toString();
    return c;
}

QList<Client> ClientRepository::all() {
    QList<Client> list;
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.exec("SELECT * FROM clients ORDER BY name");
    while (q.next())
        list.append(rowToClient(q));
    return list;
}

ResultOr<Client> ClientRepository::findById(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT * FROM clients WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next())
        return {true, {}, rowToClient(q)};
    return {false, "Cliente no encontrado", Client{}};
}

Result ClientRepository::save(const Client &client) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("INSERT INTO clients (name, address, phone) VALUES (?, ?, ?)");
    q.addBindValue(client.name);
    q.addBindValue(client.address);
    q.addBindValue(client.phone);
    if (!q.exec()) {
        qDebug() << "Error guardando cliente:" << q.lastError().text();
        return Result::fail("Error al guardar cliente");
    }
    return Result::ok();
}

Result ClientRepository::update(const Client &client) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("UPDATE clients SET name=?, address=?, phone=? WHERE id=?");
    q.addBindValue(client.name);
    q.addBindValue(client.address);
    q.addBindValue(client.phone);
    q.addBindValue(client.id);
    if (!q.exec()) {
        qDebug() << "Error actualizando cliente:" << q.lastError().text();
        return Result::fail("Error al actualizar cliente");
    }
    if (q.numRowsAffected() == 0)
        return Result::fail("Cliente no encontrado");
    return Result::ok();
}

Result ClientRepository::remove(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("DELETE FROM clients WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        if (q.lastError().text().contains("foreign key")) {
            qDebug() << "Cliente tiene facturas:" << q.lastError().text();
            return Result::fail("No se puede eliminar: el cliente tiene facturas");
        }
        qDebug() << "Error eliminando cliente:" << q.lastError().text();
        return Result::fail("Error al eliminar cliente");
    }
    if (q.numRowsAffected() == 0)
        return Result::fail("Cliente no encontrado");
    return Result::ok();
}
