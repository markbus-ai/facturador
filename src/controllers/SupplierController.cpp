#include "SupplierController.h"
#include "core/Validation.h"
#include "database/SupplierRepository.h"

SupplierController::SupplierController(const IAccessContract &access)
    : m_access(access) {}

QList<Supplier> SupplierController::allSuppliers() {
    if (!m_access.canManageSuppliers())
        return {};
    return SupplierRepository::instance().all();
}

ResultOr<Supplier> SupplierController::findSupplier(int id) {
    if (!m_access.canManageSuppliers())
        return {false, "No tiene permisos para gestionar proveedores", Supplier{}};
    if (!Validation::isIdValid(id))
        return {false, "ID de proveedor inv\u00e1lido", Supplier{}};
    return SupplierRepository::instance().findById(id);
}

Result SupplierController::addSupplier(const QString &name,
                                       const QString &contact,
                                       const QString &phone,
                                       const QString &email,
                                       const QString &address,
                                       const QString &cuit) {
    if (!m_access.canManageSuppliers())
        return Result::fail("No tiene permisos para gestionar proveedores");

    if (!Validation::isNotEmpty(name))
        return Result::fail("El nombre del proveedor es obligatorio");

    if (!Validation::isLengthInRange(name, 1, 100))
        return Result::fail("El nombre no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(contact, 0, 100))
        return Result::fail("El contacto no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(address, 0, 200))
        return Result::fail("La direcci\u00f3n no puede superar los 200 caracteres");

    if (!phone.trimmed().isEmpty() && !Validation::isValidPhone(phone))
        return Result::fail("El tel\u00e9fono debe tener al menos 7 d\u00edgitos");

    if (!email.trimmed().isEmpty() &&
        !email.trimmed().contains('@')) {
        return Result::fail("El email no es v\u00e1lido");
    }

    if (!cuit.trimmed().isEmpty()) {
        QString cuitClean = cuit.trimmed();
        cuitClean.remove('-');
        bool validCuit = cuitClean.length() == 11;
        for (const QChar &c : cuitClean)
            if (!c.isDigit()) { validCuit = false; break; }
        if (!validCuit)
            return Result::fail("El CUIT debe tener formato XX-XXXXXXXX-X");
    }

    Supplier s;
    s.name = name.trimmed();
    s.contact = contact.trimmed();
    s.phone = Validation::sanitizePhone(phone);
    s.email = email.trimmed();
    s.address = address.trimmed();
    s.cuit = cuit.trimmed();
    return SupplierRepository::instance().save(s);
}

Result SupplierController::updateSupplier(const Supplier &supplier) {
    if (!m_access.canManageSuppliers())
        return Result::fail("No tiene permisos para gestionar proveedores");

    if (!Validation::isIdValid(supplier.id))
        return Result::fail("ID de proveedor inv\u00e1lido");

    if (!Validation::isNotEmpty(supplier.name))
        return Result::fail("El nombre del proveedor es obligatorio");

    if (!Validation::isLengthInRange(supplier.name, 1, 100))
        return Result::fail("El nombre no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(supplier.contact, 0, 100))
        return Result::fail("El contacto no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(supplier.address, 0, 200))
        return Result::fail("La direcci\u00f3n no puede superar los 200 caracteres");

    if (!supplier.phone.trimmed().isEmpty() && !Validation::isValidPhone(supplier.phone))
        return Result::fail("El tel\u00e9fono debe tener al menos 7 d\u00edgitos");

    if (!supplier.email.trimmed().isEmpty() &&
        !supplier.email.trimmed().contains('@')) {
        return Result::fail("El email no es v\u00e1lido");
    }

    if (!supplier.cuit.trimmed().isEmpty()) {
        QString cuitClean = supplier.cuit.trimmed();
        cuitClean.remove('-');
        bool validCuit = cuitClean.length() == 11;
        for (const QChar &c : cuitClean)
            if (!c.isDigit()) { validCuit = false; break; }
        if (!validCuit)
            return Result::fail("El CUIT debe tener formato XX-XXXXXXXX-X");
    }

    return SupplierRepository::instance().update(supplier);
}

Result SupplierController::removeSupplier(int id) {
    if (!m_access.canManageSuppliers())
        return Result::fail("No tiene permisos para gestionar proveedores");

    if (!Validation::isIdValid(id))
        return Result::fail("ID de proveedor inv\u00e1lido");

    return SupplierRepository::instance().remove(id);
}
