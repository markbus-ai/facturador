#pragma once
#include "core/Result.h"
#include "database/UserRepository.h"
#include "core/IAccessContract.h"
#include <memory>
#include <QList>

class AuthController {
public:
    AuthController();

    ResultOr<UserData> login(const QString &username, const QString &password);
    Result registerUser(const QString &username, const QString &password,
                        const QString &confirmPassword,
                        const QString &role = "user");
    ResultOr<UserData> findUser(const QString &username);
    QList<UserData> allUsers();
    Result changeRole(int userId, const QString &newRole);
    Result deleteUser(int userId);

    const IAccessContract &access() const;
    const UserData &currentUser() const;
    bool isLoggedIn() const;

private:
    UserData m_currentUser;
    std::unique_ptr<IAccessContract> m_access;
};
