#pragma once
#include "core/Client.h"
#include "core/Result.h"
#include "core/IAccessContract.h"
#include <QList>

class ClientController {
public:
    explicit ClientController(const IAccessContract &access);

    QList<Client> allClients();
    ResultOr<Client> findClient(int id);
    Result addClient(const QString &name, const QString &address,
                     const QString &phone);
    Result updateClient(const Client &client);
    Result removeClient(int id);

private:
    const IAccessContract &m_access;
};
