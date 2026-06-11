#include "InventoryController.h"
#include "core/Validation.h"
#include "database/ProductRepository.h"

InventoryController::InventoryController(const IAccessContract &access)
    : m_access(access) {}

QList<Product> InventoryController::allProducts() {
    if (!m_access.canQueryProducts())
        return {};
    return ProductRepository::instance().all();
}

ResultOr<Product> InventoryController::findProduct(int id) {
    if (!Validation::isIdValid(id))
        return {false, "ID de producto inv\u00e1lido", Product{}};
    return ProductRepository::instance().findById(id);
}

Result InventoryController::addProduct(const QString &name,
                                       const QString &description,
                                       double price, int stock, int minStock) {
    if (!m_access.canManageProducts())
        return Result::fail("No tiene permisos para gestionar productos");

    if (!Validation::isNotEmpty(name))
        return Result::fail("El nombre del producto es obligatorio");

    if (!Validation::isLengthInRange(name, 1, 100))
        return Result::fail("El nombre no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(description, 0, 500))
        return Result::fail("La descripci\u00f3n no puede superar los 500 caracteres");

    if (!Validation::isInRange(price, 0.01, Validation::MAX_PRICE))
        return Result::fail("El precio debe estar entre 0.01 y 999999.99");

    if (!Validation::isNonNegative(stock))
        return Result::fail("El stock no puede ser negativo");

    if (!Validation::isNonNegative(minStock))
        return Result::fail("El stock m\u00ednimo no puede ser negativo");

    Product p;
    p.name = name.trimmed();
    p.description = description.trimmed();
    p.price = price;
    p.stock = stock;
    p.minStock = minStock;
    return ProductRepository::instance().save(p);
}

Result InventoryController::updateProduct(const Product &product) {
    if (!m_access.canManageProducts())
        return Result::fail("No tiene permisos para gestionar productos");

    if (!Validation::isIdValid(product.id))
        return Result::fail("ID de producto inv\u00e1lido");

    if (!Validation::isNotEmpty(product.name))
        return Result::fail("El nombre del producto es obligatorio");

    if (!Validation::isLengthInRange(product.name, 1, 100))
        return Result::fail("El nombre no puede superar los 100 caracteres");

    if (!Validation::isLengthInRange(product.description, 0, 500))
        return Result::fail("La descripci\u00f3n no puede superar los 500 caracteres");

    if (!Validation::isInRange(product.price, 0.01, Validation::MAX_PRICE))
        return Result::fail("El precio debe estar entre 0.01 y 999999.99");

    if (!Validation::isNonNegative(product.stock))
        return Result::fail("El stock no puede ser negativo");

    if (!Validation::isNonNegative(product.minStock))
        return Result::fail("El stock m\u00ednimo no puede ser negativo");

    return ProductRepository::instance().update(product);
}

Result InventoryController::removeProduct(int id) {
    if (!m_access.canManageProducts())
        return Result::fail("No tiene permisos para gestionar productos");

    if (!Validation::isIdValid(id))
        return Result::fail("ID de producto inv\u00e1lido");

    return ProductRepository::instance().remove(id);
}

QList<Product> InventoryController::lowStockReport() {
    if (!m_access.canGenerateReports())
        return {};

    return ProductRepository::instance().lowStock();
}
