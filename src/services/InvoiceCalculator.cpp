#include "InvoiceCalculator.h"

InvoiceTotals InvoiceCalculator::calculate(double subtotal,
                                           const IDiscountStrategy &discount) {
  InvoiceTotals totals;
  totals.subtotal = subtotal;
  totals.discountAmount = discount.calculateDiscount(subtotal);
  totals.taxableAmount = subtotal - totals.discountAmount;
  totals.vat = totals.taxableAmount * IVA_RATE;
  totals.total = totals.taxableAmount + totals.vat;
  return totals;
}

InvoiceTotals InvoiceCalculator::calculateBoth(
    const QList<InvoiceItem> &items,
    const IDiscountStrategy &globalDiscount) {

  InvoiceTotals totals;
  double sumAfterPerItem = 0.0;

  for (const auto &item : items) {
    totals.subtotal += item.subtotal;
    double disc = item.itemDiscountAmount();
    totals.discountAmount += disc;
    sumAfterPerItem += item.subtotal - disc;
  }

  double globalDisc = globalDiscount.calculateDiscount(sumAfterPerItem);
  totals.discountAmount += globalDisc;
  totals.taxableAmount = sumAfterPerItem - globalDisc;
  totals.vat = totals.taxableAmount * IVA_RATE;
  totals.total = totals.taxableAmount + totals.vat;
  return totals;
}
