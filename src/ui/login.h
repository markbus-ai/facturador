#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class LoginWindow : public QDialog {
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() = default;

    QString username() const;
    QString role() const;
    int userId() const;

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;

    QString m_username;
    QString m_role;
    int m_userId = 0;

    void togglePasswordVisibility();
};
