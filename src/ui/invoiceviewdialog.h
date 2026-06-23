#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include "core/Invoice.h"

class InvoiceViewDialog : public QDialog {
    Q_OBJECT

public:
    explicit InvoiceViewDialog(const Invoice &invoice,
                               QWidget *parent = nullptr);
    ~InvoiceViewDialog() = default;

private:
    Invoice m_invoice;
    QTableWidget *tablaItems;

    void setupUi();
};
