#include "UserRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>

UserRepository &UserRepository::instance() {
    static UserRepository repo;
    return repo;
}

UserData UserRepository::rowToUser(const QSqlQuery &q) const {
    UserData u;
    u.id = q.value("id").toInt();
    u.username = q.value("username").toString();
    u.role = q.value("role").toString();
    return u;
}

static QString saltedHash(const QString &password, const QString &salt) {
    QByteArray data = (salt + password).toUtf8();
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

static QString generateSalt() {
    QByteArray salt;
    salt.resize(16);
    for (int i = 0; i < 16; ++i)
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    return QString(salt.toHex());
}

Result UserRepository::registerUser(const QString &username,
                                    const QString &password,
                                    const QString &role) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT id FROM users WHERE username = ?");
    q.addBindValue(username);
    q.exec();
    if (q.next())
        return Result::fail("El nombre de usuario ya existe");

    QString salt = generateSalt();
    QString hash = saltedHash(password, salt);
    QSqlQuery ins(db);
    ins.prepare("INSERT INTO users (username, password, role, salt) VALUES (?, ?, ?, ?)");
    ins.addBindValue(username);
    ins.addBindValue(hash);
    ins.addBindValue(role);
    ins.addBindValue(salt);

    if (!ins.exec()) {
        qDebug() << "Error registrando usuario:" << ins.lastError().text();
        return Result::fail("Error al registrar usuario");
    }
    return Result::ok();
}

ResultOr<UserData> UserRepository::validateUser(const QString &username,
                                                const QString &password) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT id, username, password, role, salt FROM users WHERE username = ?");
    q.addBindValue(username);

    if (!q.exec() || !q.next())
        return {false, "Usuario o contrase\u00f1a incorrectos", UserData{}};

    QString storedHash = q.value("password").toString();
    QString salt = q.value("salt").toString();

    QString inputHash;
    if (salt.isEmpty()) {
        inputHash = QString(QCryptographicHash::hash(
            password.toUtf8(), QCryptographicHash::Sha256).toHex());
    } else {
        inputHash = saltedHash(password, salt);
    }

    if (storedHash != inputHash)
        return {false, "Usuario o contrase\u00f1a incorrectos", UserData{}};

    UserData u;
    u.id = q.value("id").toInt();
    u.username = q.value("username").toString();
    u.role = q.value("role").toString();
    return {true, {}, u};
}

ResultOr<UserData> UserRepository::findById(int id) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT id, username, role FROM users WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next())
        return {true, {}, rowToUser(q)};
    return {false, "Usuario no encontrado", UserData{}};
}

ResultOr<UserData> UserRepository::findByUsername(const QString &username) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("SELECT id, username, role FROM users WHERE username = ?");
    q.addBindValue(username);
    if (q.exec() && q.next())
        return {true, {}, rowToUser(q)};
    return {false, "Usuario no encontrado", UserData{}};
}

QList<UserData> UserRepository::all() {
    QList<UserData> list;
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.exec("SELECT id, username, role FROM users ORDER BY id");
    while (q.next())
        list.append(rowToUser(q));
    return list;
}

Result UserRepository::changeRole(int userId, const QString &newRole) {
    auto &db = DatabaseManager::instance().database();
    QSqlQuery q(db);
    q.prepare("UPDATE users SET role = ? WHERE id = ?");
    q.addBindValue(newRole);
    q.addBindValue(userId);
    if (!q.exec())
        return Result::fail("Error al cambiar rol");
    return Result::ok();
}

Result UserRepository::remove(int userId) {
    auto &db = DatabaseManager::instance().database();
    db.transaction();

    QSqlQuery roleQ(db);
    roleQ.prepare("SELECT role FROM users WHERE id = ?");
    roleQ.addBindValue(userId);
    if (!roleQ.exec() || !roleQ.next()) {
        db.rollback();
        return Result::fail("Usuario no encontrado");
    }
    QString role = roleQ.value("role").toString();

    if (role == "admin") {
        QSqlQuery countQ(db);
        countQ.prepare("SELECT COUNT(*) FROM users WHERE role = ?");
        countQ.addBindValue("admin");
        if (countQ.exec() && countQ.next() && countQ.value(0).toInt() <= 1) {
            db.rollback();
            return Result::fail("No puede eliminar al unico admin del sistema");
        }
    }

    QSqlQuery q(db);
    q.prepare("DELETE FROM users WHERE id = ?");
    q.addBindValue(userId);
    if (!q.exec()) {
        qDebug() << "Error eliminando usuario:" << q.lastError().text();
        db.rollback();
        return Result::fail("Error al eliminar usuario");
    }
    db.commit();
    return Result::ok();
}
