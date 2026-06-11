#pragma once
#include <QString>

struct Result {
    bool success;
    QString errorMessage;

    static Result ok() { return {true, {}}; }
    static Result fail(const QString &msg) { return {false, msg}; }
};

template <typename T>
struct ResultOr {
    bool success;
    QString message;
    T value;
};
