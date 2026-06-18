#include "login.h"
#include "registerwindows.h"
#include "controllers/AuthController.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QAction>
#include <QApplication>
#include <QStyle>

LoginWindow::LoginWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Facturador");
    setAccessibleName("Inicio de sesi\u00f3n");
    setMinimumSize(380, 460);
    resize(380, 460);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(0);

    QLabel *titleLabel = new QLabel("facturador");
    titleLabel->setStyleSheet(
        "font-size: 28px; font-weight: 300; color: #1d1d1f; letter-spacing: 1px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel("Sistema de facturaci\u00f3n");
    subtitleLabel->setStyleSheet("font-size: 13px; color: #5e5e63; margin-top: 4px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(subtitleLabel);

    layout->addSpacing(36);

    QLabel *usernameLabel = new QLabel("Usuario");
    usernameLabel->setStyleSheet("font-size: 12px; font-weight: 500; color: #6e6e73; "
                                 "letter-spacing: 0.3px; margin-bottom: 4px;");
    layout->addWidget(usernameLabel);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Ingrese su usuario");
    usernameEdit->setAccessibleName("Usuario");
    layout->addWidget(usernameEdit);
    usernameLabel->setBuddy(usernameEdit);

    layout->addSpacing(16);

    QLabel *passwordLabel = new QLabel("Contrase\u00f1a");
    passwordLabel->setStyleSheet("font-size: 12px; font-weight: 500; color: #6e6e73; "
                                 "letter-spacing: 0.3px; margin-bottom: 4px;");
    layout->addWidget(passwordLabel);

    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Ingrese su contrase\u00f1a");

    QAction *toggleAction = passwordEdit->addAction(
        QIcon::fromTheme("view-visible",
                         QIcon::fromTheme("dialog-password",
                                          QApplication::style()->standardIcon(QStyle::SP_ArrowUp))),
        QLineEdit::TrailingPosition);
    toggleAction->setCheckable(true);
    toggleAction->setText("Mostrar contrase\u00f1a");
    toggleAction->setToolTip("Alternar visibilidad de la contrase\u00f1a");
    connect(toggleAction, &QAction::triggered, this,
            &LoginWindow::togglePasswordVisibility);

    passwordEdit->setAccessibleName("Contrase\u00f1a");
    layout->addWidget(passwordEdit);
    passwordLabel->setBuddy(passwordEdit);

    layout->addSpacing(24);

    loginButton = new QPushButton("Ingresar");
    loginButton->setObjectName("btnPrimary");
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setMinimumHeight(42);
    loginButton->setDefault(true);
    layout->addWidget(loginButton);

    layout->addSpacing(12);

    QHBoxLayout *registerRow = new QHBoxLayout();
    registerRow->setAlignment(Qt::AlignCenter);

    QLabel *noAccountLabel = new QLabel("\u00bfNo ten\u00e9s cuenta?");
    noAccountLabel->setStyleSheet("font-size: 12px; color: #5e5e63;");
    registerRow->addWidget(noAccountLabel);

    registerButton = new QPushButton("Crear cuenta");
    registerButton->setObjectName("btnText");
    registerButton->setCursor(Qt::PointingHandCursor);
    registerRow->addWidget(registerButton);

    layout->addLayout(registerRow);
    layout->addStretch();

    connect(passwordEdit, &QLineEdit::returnPressed, loginButton, &QPushButton::click);
    connect(loginButton, &QPushButton::clicked, this, [this]() {
        AuthController auth;
        auto result = auth.login(usernameEdit->text().trimmed(), passwordEdit->text());
        if (result.success) {
            m_username = result.value.username;
            m_role = result.value.role;
            m_userId = result.value.id;
            accept();
        } else {
            QMessageBox::warning(this, "Error", result.message);
        }
    });

    connect(registerButton, &QPushButton::clicked, this, [this]() {
        RegisterWindow reg(this);
        if (reg.exec() == QDialog::Accepted && !reg.registeredUsername().isEmpty()) {
            usernameEdit->setText(reg.registeredUsername());
            passwordEdit->setFocus();
        }
    });

    usernameEdit->setFocus();
}

void LoginWindow::togglePasswordVisibility() {
    bool hidden = passwordEdit->echoMode() == QLineEdit::Password;
    passwordEdit->setEchoMode(
        hidden ? QLineEdit::Normal : QLineEdit::Password);
    QAction *a = passwordEdit->actions().value(0);
    if (a) {
        a->setChecked(!hidden);
        a->setText(hidden ? "Ocultar contrase\u00f1a" : "Mostrar contrase\u00f1a");
    }
}

QString LoginWindow::username() const { return m_username; }
QString LoginWindow::role() const { return m_role; }
int LoginWindow::userId() const { return m_userId; }
