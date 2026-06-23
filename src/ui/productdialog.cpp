#include "productdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QMap>

ProductDialog::ProductDialog(SupplierController &suppliers,
                             QWidget *parent)
    : QDialog(parent), m_suppliers(suppliers) {
    setupUi(false);
}

ProductDialog::ProductDialog(const Product &product,
                             SupplierController &suppliers,
                             QWidget *parent)
    : QDialog(parent), m_product(product), m_suppliers(suppliers) {
    setupUi(true);
}

void ProductDialog::setupUi(bool editing) {
    setWindowTitle(editing ? "Editar Producto" : "Nuevo Producto");
    setAccessibleName(windowTitle());
    setMinimumWidth(420);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    QLabel *title = new QLabel(windowTitle());
    title->setStyleSheet("font-size: 18px; font-weight: 300; color: #1d1d1f;");
    mainLayout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(8);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_name = new QLineEdit(m_product.name);
    m_name->setPlaceholderText("Nombre del producto");
    m_name->setMaxLength(100);
    form->addRow("Nombre:", m_name);

    m_desc = new QLineEdit(m_product.description);
    m_desc->setPlaceholderText("Descripcion (opcional)");
    m_desc->setMaxLength(500);
    form->addRow("Descripcion:", m_desc);

    m_price = new QDoubleSpinBox();
    m_price->setRange(0.01, 999999.99);
    m_price->setDecimals(2);
    m_price->setPrefix("$ ");
    m_price->setValue(m_product.price > 0 ? m_product.price : 0.01);
    form->addRow("Precio:", m_price);

    m_stock = new QSpinBox();
    m_stock->setRange(0, 999999);
    m_stock->setValue(m_product.stock);
    form->addRow("Stock:", m_stock);

    m_minStock = new QSpinBox();
    m_minStock->setRange(0, 999999);
    m_minStock->setValue(m_product.minStock);
    form->addRow("Stock Minimo:", m_minStock);

    m_supplier = new QComboBox();
    populateSuppliers(m_product.supplierId);
    form->addRow("Proveedor:", m_supplier);

    mainLayout->addLayout(form);
    mainLayout->addSpacing(8);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton *btnCancel = new QPushButton("Cancelar");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setAutoDefault(false);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(btnCancel);

    QPushButton *btnSave = new QPushButton(editing ? "Guardar Cambios" : "Crear Producto");
    btnSave->setObjectName("btnPrimary");
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setDefault(true);
    connect(btnSave, &QPushButton::clicked, this, [this]() {
        if (m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validacion",
                                 "El nombre del producto es obligatorio");
            m_name->setFocus();
            return;
        }
        m_product.name = m_name->text().trimmed();
        m_product.description = m_desc->text().trimmed();
        m_product.price = m_price->value();
        m_product.stock = m_stock->value();
        m_product.minStock = m_minStock->value();
        m_product.supplierId = m_supplier->currentData().toInt();
        accept();
    });
    btnRow->addWidget(btnSave);

    mainLayout->addLayout(btnRow);
}

void ProductDialog::populateSuppliers(int selectedId) {
    m_supplier->addItem("(Sin proveedor)", 0);
    int selectIdx = 0;
    int idx = 1;
    auto suppliers = m_suppliers.allSuppliers();
    for (const auto &s : suppliers) {
        m_supplier->addItem(s.name, s.id);
        if (s.id == selectedId)
            selectIdx = idx;
        ++idx;
    }
    m_supplier->setCurrentIndex(selectIdx);
}

Product ProductDialog::product() const {
    return m_product;
}

void ProductDialog::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        QWidget *focused = focusWidget();
        if (focused && !focused->inherits("QPushButton")) {
            event->accept();
            return;
        }
    }
    QDialog::keyPressEvent(event);
}
