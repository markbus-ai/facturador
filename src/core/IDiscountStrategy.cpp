#include "IDiscountStrategy.h"
#include <QtGlobal>

PercentageDiscount::PercentageDiscount(double rate)
    : m_rate(qBound(0.0, rate, 1.0)) {}

QString PercentageDiscount::name() const {
  return QString("Descuento %1%").arg(m_rate * 100, 0, 'f', 0);
}

double PercentageDiscount::calculateDiscount(double subtotal) const {
  return subtotal * m_rate;
}

NominalDiscount::NominalDiscount(double amount) : m_amount(qMax(0.0, amount)) {}

QString NominalDiscount::name() const {
  return QString("Descuento fijo $%1").arg(m_amount, 0, 'f', 2);
}

double NominalDiscount::calculateDiscount(double subtotal) const {
  return qMin(m_amount, subtotal);
}

std::unique_ptr<IDiscountStrategy> DiscountFactory::create(const QString &type,
                                                           double value) {
  if (type == "percentage")
    return std::make_unique<PercentageDiscount>(value / 100.0);
  if (type == "nominal")
    return std::make_unique<NominalDiscount>(value);
  return std::make_unique<NoDiscount>();
}
