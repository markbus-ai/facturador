#pragma once
#include <QString>
#include "core/Result.h"

class QSqlQuery;

struct UserData {
    int id = 0;
    QString username;
    QString role;
};

class UserRepository {
public:
    static UserRepository &instance();

    Result registerUser(const QString &username, const QString &password,
                        const QString &role = "user");
    ResultOr<UserData> validateUser(const QString &username,
                                    const QString &password);
    ResultOr<UserData> findById(int id);
    ResultOr<UserData> findByUsername(const QString &username);
    QList<UserData> all();
    Result changeRole(int userId, const QString &newRole);
    Result remove(int userId);

private:
    UserRepository() = default;
    UserData rowToUser(const QSqlQuery &q) const;
};
