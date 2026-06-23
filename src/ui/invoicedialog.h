#pragma once
#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QIntValidator>
#include "controllers/BillingController.h"

#include "core/IDiscountStrategy.h"

class InvoiceDialog : public QDialog {
    Q_OBJECT

public:
    InvoiceDialog(BillingController &billing, int userId,
                  QWidget *parent = nullptr);
    ~InvoiceDialog() = default;

private:
    BillingController &m_billing;
    int m_userId;

    QComboBox *cmbCliente;
    QComboBox *cmbProducto;
    QSpinBox *spnCantidad;
    QTableWidget *tablaItems;
    QLabel *lblSubtotal;
    QLabel *lblDescuento;
    QLabel *lblVat;
    QLabel *lblTotal;

    QGroupBox *grpDescuentoGlobal;
    QComboBox *cmbTipoDescuento;
    QDoubleSpinBox *spnValorDescuento;

    QGroupBox *grpDescuentoItem;
    QComboBox *cmbItemTipoDesc;
    QDoubleSpinBox *spnItemValorDesc;

    QMetaObject::Connection m_cellChangedConn;
    QIntValidator *m_qtyValidator = nullptr;

    void addItemRow();
    void removeItemRow();
    void updateTotals();
    void saveInvoice();
    void syncItemDiscountControls();
    std::unique_ptr<IDiscountStrategy> crearEstrategiaGlobal() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
};
