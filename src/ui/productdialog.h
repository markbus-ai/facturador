#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include "core/Product.h"
#include "controllers/SupplierController.h"

class ProductDialog : public QDialog {
    Q_OBJECT

public:
    ProductDialog(SupplierController &suppliers,
                  QWidget *parent = nullptr);
    ProductDialog(const Product &product, SupplierController &suppliers,
                  QWidget *parent = nullptr);

    Product product() const;

private:
    QLineEdit *m_name;
    QLineEdit *m_desc;
    QDoubleSpinBox *m_price;
    QSpinBox *m_stock;
    QSpinBox *m_minStock;
    QComboBox *m_supplier;

    Product m_product;
    SupplierController &m_suppliers;

    void setupUi(bool editing);
    void populateSuppliers(int selectedId = 0);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};
