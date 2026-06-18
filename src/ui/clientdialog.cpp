#include "clientdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

ClientDialog::ClientDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi(false);
}

ClientDialog::ClientDialog(const Client &client, QWidget *parent)
    : QDialog(parent), m_client(client) {
    setupUi(true);
}

void ClientDialog::setupUi(bool editing) {
    setWindowTitle(editing ? "Editar Cliente" : "Nuevo Cliente");
    setAccessibleName(windowTitle());
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    QLabel *title = new QLabel(windowTitle());
    title->setStyleSheet("font-size: 18px; font-weight: 300; color: #1d1d1f;");
    mainLayout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(8);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_name = new QLineEdit(m_client.name);
    m_name->setPlaceholderText("Nombre del cliente");
    m_name->setMaxLength(200);
    form->addRow("Nombre:", m_name);

    m_address = new QLineEdit(m_client.address);
    m_address->setPlaceholderText("Direccion (opcional)");
    m_address->setMaxLength(5000);
    form->addRow("Direccion:", m_address);

    m_phone = new QLineEdit(m_client.phone);
    m_phone->setPlaceholderText("Telefono (opcional)");
    form->addRow("Telefono:", m_phone);

    mainLayout->addLayout(form);
    mainLayout->addSpacing(8);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton *btnCancel = new QPushButton("Cancelar");
    btnCancel->setCursor(Qt::PointingHandCursor);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(btnCancel);

    QPushButton *btnSave = new QPushButton(editing ? "Guardar Cambios" : "Crear Cliente");
    btnSave->setObjectName("btnPrimary");
    btnSave->setCursor(Qt::PointingHandCursor);
    connect(btnSave, &QPushButton::clicked, this, [this]() {
        if (m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validacion",
                                 "El nombre del cliente es obligatorio");
            m_name->setFocus();
            return;
        }
        m_client.name = m_name->text().trimmed();
        m_client.address = m_address->text().trimmed();
        m_client.phone = m_phone->text().trimmed();
        accept();
    });
    btnRow->addWidget(btnSave);

    mainLayout->addLayout(btnRow);
}

Client ClientDialog::client() const {
    return m_client;
}
