#pragma once
#include "core/IDiscountStrategy.h"
#include "core/Invoice.h"

struct InvoiceTotals {
    double subtotal = 0.0;
    double discountAmount = 0.0;
    double taxableAmount = 0.0;
    double vat = 0.0;
    double total = 0.0;
};

class InvoiceCalculator {
public:
    static constexpr double IVA_RATE = 0.21;

    static InvoiceTotals calculate(double subtotal,
                                   const IDiscountStrategy &discount);

    static InvoiceTotals calculateBoth(const QList<InvoiceItem> &items,
                                       const IDiscountStrategy &globalDiscount);
};

