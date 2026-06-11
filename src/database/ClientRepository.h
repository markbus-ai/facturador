#pragma once
#include <QList>
#include "core/Client.h"
#include "core/Result.h"

class QSqlQuery;

class ClientRepository {
public:
    static ClientRepository &instance();

    QList<Client> all();
    ResultOr<Client> findById(int id);
    Result save(const Client &client);
    Result update(const Client &client);
    Result remove(int id);

private:
    ClientRepository() = default;
    Client rowToClient(const QSqlQuery &q) const;
};
