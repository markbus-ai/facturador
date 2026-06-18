#pragma once
#include <QString>
#include <QList>

struct InvoiceItem {
    int productId = 0;
    QString productName;
    int quantity = 0;
    double unitPrice = 0.0;
    double subtotal = 0.0;
    QString discountType;
    double discountValue = 0.0;

    double discountedSubtotal() const {
        if (discountType == "percentage")
            return qMax(0.0, subtotal * (1.0 - discountValue / 100.0));
        if (discountType == "nominal")
            return qMax(0.0, subtotal - discountValue);
        return subtotal;
    }

    double itemDiscountAmount() const {
        return subtotal - discountedSubtotal();
    }
};

struct Invoice {
    int id = 0;
    int clientId = 0;
    QString clientName;
    int userId = 0;
    QString userName;
    QString date;
    double subtotal = 0.0;
    double discountAmount = 0.0;
    QString discountStrategyName;
    double vat = 0.0;
    double total = 0.0;
    QList<InvoiceItem> items;
};
