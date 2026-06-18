#include "supplierdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

SupplierDialog::SupplierDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi(false);
}

SupplierDialog::SupplierDialog(const Supplier &supplier, QWidget *parent)
    : QDialog(parent), m_supplier(supplier) {
    setupUi(true);
}

void SupplierDialog::setupUi(bool editing) {
    setWindowTitle(editing ? "Editar Proveedor" : "Nuevo Proveedor");
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

    m_name = new QLineEdit(m_supplier.name);
    m_name->setPlaceholderText("Razon social del proveedor");
    m_name->setMaxLength(200);
    form->addRow("Nombre:", m_name);

    m_contact = new QLineEdit(m_supplier.contact);
    m_contact->setPlaceholderText("Persona de contacto (opcional)");
    m_contact->setMaxLength(100);
    form->addRow("Contacto:", m_contact);

    m_phone = new QLineEdit(m_supplier.phone);
    m_phone->setPlaceholderText("Telefono (opcional)");
    m_phone->setMaxLength(50);
    form->addRow("Telefono:", m_phone);

    m_email = new QLineEdit(m_supplier.email);
    m_email->setPlaceholderText("Email (opcional)");
    m_email->setMaxLength(100);
    form->addRow("Email:", m_email);

    m_address = new QLineEdit(m_supplier.address);
    m_address->setPlaceholderText("Direccion (opcional)");
    m_address->setMaxLength(5000);
    form->addRow("Direccion:", m_address);

    m_cuit = new QLineEdit(m_supplier.cuit);
    m_cuit->setPlaceholderText("CUIT (opcional)");
    m_cuit->setMaxLength(20);
    form->addRow("CUIT:", m_cuit);

    mainLayout->addLayout(form);
    mainLayout->addSpacing(8);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton *btnCancel = new QPushButton("Cancelar");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setAutoDefault(false);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(btnCancel);

    QPushButton *btnSave = new QPushButton(editing ? "Guardar Cambios" : "Crear Proveedor");
    btnSave->setObjectName("btnPrimary");
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setDefault(true);
    connect(btnSave, &QPushButton::clicked, this, [this]() {
        if (m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validacion",
                                 "El nombre del proveedor es obligatorio");
            m_name->setFocus();
            return;
        }
        m_supplier.name = m_name->text().trimmed();
        m_supplier.contact = m_contact->text().trimmed();
        m_supplier.phone = m_phone->text().trimmed();
        m_supplier.email = m_email->text().trimmed();
        m_supplier.address = m_address->text().trimmed();
        m_supplier.cuit = m_cuit->text().trimmed();
        accept();
    });
    btnRow->addWidget(btnSave);

    mainLayout->addLayout(btnRow);
}

Supplier SupplierDialog::supplier() const {
    return m_supplier;
}
