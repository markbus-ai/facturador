#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QDebug>
#include <QCryptographicHash>

DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

static bool openDb(QSqlDatabase &db, QStringList &errors) {
    if (db.open())
        return true;
    errors << QString("%1 @ %2: %3")
                  .arg(db.userName(), db.hostName(), db.lastError().text());
    return false;
}

static QSqlDatabase makeConn(const QString &name, const QString &host,
                             const QString &dbName, const QString &socket) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", name);
    db.setHostName(host);
    db.setUserName("facturador");
    db.setPassword("facturador123");
    db.setDatabaseName(dbName);
    if (!socket.isEmpty())
        db.setConnectOptions("UNIX_SOCKET=" + socket);
    return db;
}

static QStringList sockets() {
    return {"/var/run/mysqld/mysqld.sock",
            "/var/lib/mysql/mysql.sock",
            "/tmp/mysql.sock"};
}

DatabaseManager::DatabaseManager() {
    if (QSqlDatabase::contains("fact_connection")) {
        m_db = QSqlDatabase::database("fact_connection");
        if (!m_db.isOpen())
            m_db.open();
        return;
    }

    QStringList errors;

    if (!QSqlDatabase::drivers().contains("QMYSQL")) {
        qDebug() << "Driver QMYSQL no disponible";
        return;
    }

    // Bootstrap: crear schema si no existe
    {
        QSqlDatabase boot = makeConn("bootstrap", "localhost", "", "");
        if (openDb(boot, errors)) {
            QSqlQuery q(boot);
            if (!q.exec("CREATE DATABASE IF NOT EXISTS facturador "
                        "DEFAULT CHARACTER SET utf8mb4"))
                qDebug() << "Error creando DB:" << q.lastError().text();
            q.finish();
            boot.close();
        }
        boot = QSqlDatabase();
        QSqlDatabase::removeDatabase("bootstrap");
    }

    // Conexion principal: probar localhost, socket, 127.0.0.1
    m_db = QSqlDatabase::addDatabase("QMYSQL", "fact_connection");

    for (const QString &host : {"localhost", "127.0.0.1"}) {
        m_db.setHostName(host);
        m_db.setUserName("facturador");
        m_db.setPassword("facturador123");
        m_db.setDatabaseName("facturador");
        if (openDb(m_db, errors)) {
            createTables();
            seedData();
            return;
        }
    }

    for (const QString &sock : sockets()) {
        m_db.setHostName("localhost");
        m_db.setUserName("facturador");
        m_db.setPassword("facturador123");
        m_db.setDatabaseName("facturador");
        m_db.setConnectOptions("UNIX_SOCKET=" + sock);
        if (openDb(m_db, errors)) {
            createTables();
            seedData();
            return;
        }
    }

    qDebug() << "Conexion fallida:";
    for (const QString &e : errors)
        qDebug() << "  " << e;
}

QSqlDatabase &DatabaseManager::database() {
    return m_db;
}

bool DatabaseManager::isConnected() const {
    return m_db.isOpen();
}

void DatabaseManager::createTables() {
    QSqlQuery q(m_db);

    auto execCheck = [](QSqlQuery &q, const QString &sql, const char *label) {
        if (!q.exec(sql))
            qDebug() << "Error creando" << label << ":" << q.lastError().text();
    };

    execCheck(q,
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  username VARCHAR(100) UNIQUE NOT NULL,"
        "  password VARCHAR(255) NOT NULL,"
        "  role VARCHAR(20) NOT NULL DEFAULT 'user',"
        "  salt VARCHAR(32) DEFAULT ''"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "users");

    execCheck(q,
        "CREATE TABLE IF NOT EXISTS suppliers ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  name VARCHAR(200) NOT NULL,"
        "  contact VARCHAR(100) DEFAULT '',"
        "  phone VARCHAR(50) DEFAULT '',"
        "  email VARCHAR(100) DEFAULT '',"
        "  address TEXT DEFAULT '',"
        "  cuit VARCHAR(20) DEFAULT ''"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "suppliers");

    execCheck(q,
        "CREATE TABLE IF NOT EXISTS products ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  name VARCHAR(200) NOT NULL,"
        "  description TEXT DEFAULT '',"
        "  price DECIMAL(10,2) NOT NULL DEFAULT 0.00,"
        "  stock INT NOT NULL DEFAULT 0,"
        "  min_stock INT NOT NULL DEFAULT 0,"
        "  supplier_id INT DEFAULT NULL,"
        "  CONSTRAINT stock_non_negative CHECK (stock >= 0),"
        "  FOREIGN KEY (supplier_id) REFERENCES suppliers(id) ON DELETE SET NULL"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "products");

    execCheck(q,
        "CREATE TABLE IF NOT EXISTS clients ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  name VARCHAR(200) NOT NULL,"
        "  address TEXT DEFAULT '',"
        "  phone VARCHAR(50) DEFAULT ''"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "clients");

    execCheck(q,
        "CREATE TABLE IF NOT EXISTS invoices ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  client_id INT NOT NULL,"
        "  user_id INT NOT NULL,"
        "  date DATE NOT NULL,"
        "  subtotal DECIMAL(10,2) NOT NULL DEFAULT 0.00,"
        "  discount_amount DECIMAL(10,2) NOT NULL DEFAULT 0.00,"
        "  discount_strategy VARCHAR(50) DEFAULT '',"
        "  vat DECIMAL(10,2) NOT NULL DEFAULT 0.00,"
        "  total DECIMAL(10,2) NOT NULL DEFAULT 0.00,"
        "  FOREIGN KEY (client_id) REFERENCES clients(id) ON DELETE RESTRICT,"
        "  FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE RESTRICT"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "invoices");

    execCheck(q,
        "CREATE TABLE IF NOT EXISTS invoice_items ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  invoice_id INT NOT NULL,"
        "  product_id INT NOT NULL,"
        "  product_name VARCHAR(200) NOT NULL,"
        "  quantity INT NOT NULL DEFAULT 1,"
        "  unit_price DECIMAL(10,2) NOT NULL DEFAULT 0.00,"
        "  subtotal DECIMAL(10,2) NOT NULL DEFAULT 0.00,"
        "  discount_type VARCHAR(20) DEFAULT 'none',"
        "  discount_value DECIMAL(10,2) DEFAULT 0.00,"
        "  FOREIGN KEY (invoice_id) REFERENCES invoices(id) ON DELETE CASCADE,"
        "  FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "invoice_items");

    // Migrations for existing databases
    if (q.exec("SELECT COUNT(*) FROM information_schema.COLUMNS "
               "WHERE TABLE_SCHEMA='facturador' AND TABLE_NAME='products' "
               "AND COLUMN_NAME='supplier_id'") && q.next() && q.value(0).toInt() == 0) {
        if (!q.exec("ALTER TABLE products ADD COLUMN supplier_id INT DEFAULT NULL, "
                    "ADD FOREIGN KEY (supplier_id) REFERENCES suppliers(id) ON DELETE SET NULL"))
            qDebug() << "Error en migration supplier_id:" << q.lastError().text();
    }

    if (q.exec("SELECT COUNT(*) FROM information_schema.COLUMNS "
               "WHERE TABLE_SCHEMA='facturador' AND TABLE_NAME='users' "
               "AND COLUMN_NAME='salt'") && q.next() && q.value(0).toInt() == 0) {
        if (!q.exec("ALTER TABLE users ADD COLUMN salt VARCHAR(32) DEFAULT ''"))
            qDebug() << "Error en migration salt:" << q.lastError().text();
    }
}

void DatabaseManager::seedData() {
    QSqlQuery q(m_db);

    auto execCheck = [](QSqlQuery &q, const QString &sql, const char *label) {
        if (!q.exec(sql))
            qDebug() << "Error en seed" << label << ":" << q.lastError().text();
    };

    if (!q.exec("SELECT COUNT(*) FROM users")) return;
    if (q.next() && q.value(0).toInt() == 0) {
        QByteArray hash = QCryptographicHash::hash(
            QString("admin123").toUtf8(), QCryptographicHash::Sha256);
        QSqlQuery ins(m_db);
        ins.prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");
        ins.addBindValue("admin");
        ins.addBindValue(QString(hash.toHex()));
        ins.addBindValue("admin");
        if (!ins.exec()) qDebug() << "Error seed admin:" << ins.lastError().text();

        QByteArray userHash = QCryptographicHash::hash(
            QString("user123").toUtf8(), QCryptographicHash::Sha256);
        ins.prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");
        ins.addBindValue("user");
        ins.addBindValue(QString(userHash.toHex()));
        ins.addBindValue("user");
        if (!ins.exec()) qDebug() << "Error seed user:" << ins.lastError().text();

        QByteArray repoHash = QCryptographicHash::hash(
            QString("repo123").toUtf8(), QCryptographicHash::Sha256);
        ins.prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");
        ins.addBindValue("repositor");
        ins.addBindValue(QString(repoHash.toHex()));
        ins.addBindValue("repositor");
        if (!ins.exec()) qDebug() << "Error seed repositor:" << ins.lastError().text();
    }

    if (!q.exec("SELECT COUNT(*) FROM clients")) return;
    if (q.next() && q.value(0).toInt() == 0) {
        execCheck(q, "INSERT INTO clients (name, address, phone) VALUES "
                  "('Consumidor Final', '', '')", "cliente cf");
        execCheck(q, "INSERT INTO clients (name, address, phone) VALUES "
                  "('Cliente Ejemplo 1', 'Calle Falsa 123', '555-0001')", "cliente 1");
        execCheck(q, "INSERT INTO clients (name, address, phone) VALUES "
                  "('Cliente Ejemplo 2', 'Av. Siempre Viva 742', '555-0002')", "cliente 2");
    }

    if (!q.exec("SELECT COUNT(*) FROM suppliers")) return;
    if (q.next() && q.value(0).toInt() == 0) {
        execCheck(q, "INSERT INTO suppliers (name, contact, phone, email, address, cuit) VALUES "
                  "('Distribuidora Mayorista S.A.', 'Carlos Gomez', '555-1000', "
                  "'ventas@distmayorista.com', 'Av. Corrientes 2000', '30-12345678-9')", "supplier 1");
        execCheck(q, "INSERT INTO suppliers (name, contact, phone, email, address, cuit) VALUES "
                  "('Proveedor Ejemplo 1', 'Maria Lopez', '555-2001', "
                  "'compras@proveedor1.com', 'Calle San Martin 500', '30-87654321-0')", "supplier 2");
        execCheck(q, "INSERT INTO suppliers (name, contact, phone, email, address, cuit) VALUES "
                  "('Proveedor Ejemplo 2', 'Juan Perez', '555-3002', "
                  "'info@proveedor2.com', 'Belgrano 1200', '30-11223344-5')", "supplier 3");
    }

    if (!q.exec("SELECT COUNT(*) FROM products")) return;
    if (q.next() && q.value(0).toInt() == 0) {
        execCheck(q, "INSERT INTO products (name, description, price, stock, min_stock, supplier_id) VALUES "
                  "('Producto A', 'Descripcion del producto A', 100.00, 50, 10, 1)", "product A");
        execCheck(q, "INSERT INTO products (name, description, price, stock, min_stock, supplier_id) VALUES "
                  "('Producto B', 'Descripcion del producto B', 250.50, 20, 5, 1)", "product B");
        execCheck(q, "INSERT INTO products (name, description, price, stock, min_stock, supplier_id) VALUES "
                  "('Producto C', 'Descripcion del producto C', 50.00, 5, 10, 2)", "product C");
    }
}
