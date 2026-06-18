#pragma once
#include <QDialog>
#include <QLineEdit>
#include "core/Client.h"

class ClientDialog : public QDialog {
    Q_OBJECT

public:
    explicit ClientDialog(QWidget *parent = nullptr);
    ClientDialog(const Client &client, QWidget *parent = nullptr);

    Client client() const;

private:
    QLineEdit *m_name;
    QLineEdit *m_address;
    QLineEdit *m_phone;

    Client m_client;

    void setupUi(bool editing);
};
