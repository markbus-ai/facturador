#pragma once
#include <QString>
#include <QSqlDatabase>

class DatabaseManager {
public:
    static DatabaseManager &instance();

    QSqlDatabase &database();
    bool isConnected() const;

private:
    DatabaseManager();
    ~DatabaseManager() = default;
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
    void createTables();
    void seedData();
};
