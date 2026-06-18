#pragma once
#include <QDialog>
#include <QLineEdit>
#include "core/Supplier.h"

class SupplierDialog : public QDialog {
    Q_OBJECT

public:
    explicit SupplierDialog(QWidget *parent = nullptr);
    SupplierDialog(const Supplier &supplier, QWidget *parent = nullptr);

    Supplier supplier() const;

private:
    QLineEdit *m_name;
    QLineEdit *m_contact;
    QLineEdit *m_phone;
    QLineEdit *m_email;
    QLineEdit *m_address;
    QLineEdit *m_cuit;

    Supplier m_supplier;

    void setupUi(bool editing);
};
