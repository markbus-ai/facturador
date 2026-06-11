#pragma once
#include <QString>
#include <memory>

class IDiscountStrategy {
public:
    virtual ~IDiscountStrategy() = default;
    virtual QString name() const = 0;
    virtual double calculateDiscount(double subtotal) const = 0;
};

class NoDiscount : public IDiscountStrategy {
public:
    QString name() const override { return "Sin descuento"; }
    double calculateDiscount(double subtotal) const override { return 0.0; }
};

class PercentageDiscount : public IDiscountStrategy {
    double m_rate;
public:
    explicit PercentageDiscount(double rate);
    QString name() const override;
    double calculateDiscount(double subtotal) const override;
    double rate() const { return m_rate; }
};

class NominalDiscount : public IDiscountStrategy {
    double m_amount;
public:
    explicit NominalDiscount(double amount);
    QString name() const override;
    double calculateDiscount(double subtotal) const override;
    double amount() const { return m_amount; }
};

class DiscountFactory {
public:
    static std::unique_ptr<IDiscountStrategy> create(const QString &type,
                                                     double value);
};
