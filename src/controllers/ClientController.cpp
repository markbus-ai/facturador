#include "ClientController.h"
#include "core/Validation.h"
#include "database/ClientRepository.h"

ClientController::ClientController(const IAccessContract &access)
    : m_access(access) {}

QList<Client> ClientController::allClients() {
    if (!m_access.canManageClients())
        return {};
    return ClientRepository::instance().all();
}

ResultOr<Client> ClientController::findClient(int id) {
    if (!m_access.canManageClients())
        return {false, "No tiene permisos para gestionar clientes", Client{}};
    if (!Validation::isIdValid(id))
        return {false, "ID de cliente inv\u00e1lido", Client{}};
    return ClientRepository::instance().findById(id);
}

Result ClientController::addClient(const QString &name, const QString &address,
                                   const QString &phone) {
    if (!m_access.canManageClients())
        return Result::fail("No tiene permisos para gestionar clientes");

    if (!Validation::isNotEmpty(name))
        return Result::fail("El nombre del cliente es obligatorio");

    if (!Validation::isLengthInRange(name, 1, 100))
        return Result::fail("El nombre no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(address, 0, 200))
        return Result::fail("La direcci\u00f3n no puede superar los 200 caracteres");

    if (!phone.trimmed().isEmpty() && !Validation::isValidPhone(phone))
        return Result::fail("El tel\u00e9fono debe tener al menos 7 d\u00edgitos (ej: 11-1234-5678)");

    Client c;
    c.name = name.trimmed();
    c.address = address.trimmed();
    c.phone = Validation::sanitizePhone(phone);
    return ClientRepository::instance().save(c);
}

Result ClientController::updateClient(const Client &client) {
    if (!m_access.canManageClients())
        return Result::fail("No tiene permisos para gestionar clientes");

    if (!Validation::isIdValid(client.id))
        return Result::fail("ID de cliente inv\u00e1lido");

    if (!Validation::isNotEmpty(client.name))
        return Result::fail("El nombre del cliente es obligatorio");

    if (!Validation::isLengthInRange(client.name, 1, 100))
        return Result::fail("El nombre no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(client.address, 0, 200))
        return Result::fail("La direcci\u00f3n no puede superar los 200 caracteres");

    if (!client.phone.trimmed().isEmpty() && !Validation::isValidPhone(client.phone))
        return Result::fail("El tel\u00e9fono debe tener al menos 7 d\u00edgitos (ej: 11-1234-5678)");

    Client c = client;
    c.phone = Validation::sanitizePhone(client.phone);
    return ClientRepository::instance().update(c);
}

Result ClientController::removeClient(int id) {
    if (!m_access.canManageClients())
        return Result::fail("No tiene permisos para gestionar clientes");

    if (!Validation::isIdValid(id))
        return Result::fail("ID de cliente inv\u00e1lido");

    return ClientRepository::instance().remove(id);
}
