#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class RegisterWindow : public QDialog {
    Q_OBJECT

public:
    RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow() = default;

    QString registeredUsername() const { return m_registeredUsername; }

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;
    QString m_registeredUsername;

    void togglePasswordVisibility();
    void toggleConfirmVisibility();
};
