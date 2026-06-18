#include "AuthController.h"
#include "core/Validation.h"

AuthController::AuthController()
    : m_access(std::make_unique<UserAccess>()) {}

ResultOr<UserData> AuthController::login(const QString &username,
                                         const QString &password) {
    if (!Validation::isNotEmpty(username))
        return {false, "El usuario es obligatorio", UserData{}};
    if (!Validation::isNotEmpty(password))
        return {false, "La contrase\u00f1a es obligatoria", UserData{}};

    auto result = UserRepository::instance().validateUser(username, password);
    if (result.success) {
        m_currentUser = result.value;
        if (m_currentUser.role == "admin")
            m_access = std::make_unique<AdminAccess>();
        else if (m_currentUser.role == "repositor")
            m_access = std::make_unique<RepositorAccess>();
        else
            m_access = std::make_unique<UserAccess>();
    }
    return result;
}

Result AuthController::registerUser(const QString &username,
                                    const QString &password,
                                    const QString &confirmPassword,
                                    const QString &role) {
    if (!Validation::isNotEmpty(username) || !Validation::isNotEmpty(password))
        return Result::fail("Todos los campos son obligatorios");

    if (!Validation::isLengthInRange(username, 3, 30))
        return Result::fail("El usuario debe tener entre 3 y 30 caracteres");

    if (!Validation::isAlphanumericWithUnderscore(username))
        return Result::fail("El usuario solo puede contener letras, n\u00fameros y gui\u00f3n bajo");

    if (password != confirmPassword)
        return Result::fail("Las contrase\u00f1as no coinciden");

    if (!Validation::isLengthInRange(password, 8, 64))
        return Result::fail("La contrase\u00f1a debe tener entre 8 y 64 caracteres");

    bool hasUpper = false, hasLower = false, hasDigit = false;
    for (const QChar &c : password) {
        if (c.isUpper()) hasUpper = true;
        if (c.isLower()) hasLower = true;
        if (c.isDigit()) hasDigit = true;
    }
    if (!hasUpper || !hasLower || !hasDigit)
        return Result::fail("La contrase\u00f1a debe tener may\u00fasculas, min\u00fasculas y n\u00fameros");

    QString actualRole = role.isEmpty() ? "user" : role;
    if (!Validation::isValidRole(actualRole))
        return Result::fail("Rol inv\u00e1lido. Use: admin, user o repositor");

    return UserRepository::instance().registerUser(username.trimmed(), password, actualRole);
}

ResultOr<UserData> AuthController::findUser(const QString &username) {
    if (!Validation::isNotEmpty(username))
        return {false, "El usuario es obligatorio", UserData{}};
    return UserRepository::instance().findByUsername(username);
}

QList<UserData> AuthController::allUsers() {
    return UserRepository::instance().all();
}

Result AuthController::changeRole(int userId, const QString &newRole) {
    if (!Validation::isIdValid(userId))
        return Result::fail("ID de usuario inv\u00e1lido");

    if (!Validation::isValidRole(newRole))
        return Result::fail("Rol inv\u00e1lido. Use: admin, user o repositor");

    return UserRepository::instance().changeRole(userId, newRole);
}

Result AuthController::deleteUser(int userId) {
    if (!Validation::isIdValid(userId))
        return Result::fail("ID de usuario inv\u00e1lido");

    return UserRepository::instance().remove(userId);
}

const IAccessContract &AuthController::access() const {
    return *m_access;
}

const UserData &AuthController::currentUser() const {
    return m_currentUser;
}

bool AuthController::isLoggedIn() const {
    return m_currentUser.id > 0;
}
