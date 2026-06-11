#pragma once
#include <QString>
#include <QRegularExpression>

namespace Validation {

inline bool isNotEmpty(const QString &s) {
    return !s.trimmed().isEmpty();
}

inline bool isLengthInRange(const QString &s, int min, int max) {
    int len = s.trimmed().length();
    return len >= min && len <= max;
}

inline bool isAlphanumericWithUnderscore(const QString &s) {
    static QRegularExpression re(R"(^[a-zA-Z0-9_]+$)");
    return re.match(s).hasMatch();
}

inline bool isValidPhone(const QString &s) {
    int digits = 0;
    for (const QChar &c : s.trimmed())
        if (c.isDigit()) ++digits;
    return digits >= 7 && digits <= 15;
}

inline QString sanitizePhone(const QString &s) {
    QString result;
    bool plusSeen = false;
    for (const QChar &c : s.trimmed()) {
        if (c.isDigit())
            result += c;
        else if (c == '+' && !plusSeen && result.isEmpty()) {
            result += c;
            plusSeen = true;
        }
    }
    return result;
}

inline constexpr double MAX_PRICE = 999999.99;

inline bool isValidRole(const QString &role) {
    return role == "admin" || role == "user" || role == "repositor";
}

inline bool isPositive(double value) {
    return value > 0;
}

inline bool isNonNegative(double value) {
    return value >= 0;
}

inline bool isInRange(double value, double min, double max) {
    return value >= min && value <= max;
}

inline bool isIdValid(int id) {
    return id > 0;
}

}
