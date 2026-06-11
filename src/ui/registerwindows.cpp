#include "registerwindows.h"
#include "controllers/AuthController.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFrame>
#include <QAction>
#include <QApplication>
#include <QStyle>

RegisterWindow::RegisterWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Crear cuenta");
    setAccessibleName("Crear cuenta");
    setMinimumSize(380, 480);
    resize(380, 480);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 36, 40, 36);
    layout->setSpacing(0);

    QLabel *titleLabel = new QLabel("Crear cuenta");
    titleLabel->setStyleSheet(
        "font-size: 22px; font-weight: 300; color: #1d1d1f;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    layout->addSpacing(28);

    QLabel *usernameLabel = new QLabel("Usuario");
    usernameLabel->setStyleSheet("font-size: 12px; font-weight: 500; color: #6e6e73; "
                                 "letter-spacing: 0.3px; margin-bottom: 4px;");
    layout->addWidget(usernameLabel);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Elija un nombre de usuario");
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
    passwordEdit->setPlaceholderText("Elija una contrase\u00f1a");
    passwordEdit->setAccessibleName("Contrase\u00f1a");

    QAction *togglePw = passwordEdit->addAction(
        QIcon::fromTheme("view-visible",
                         QIcon::fromTheme("dialog-password",
                                          QApplication::style()->standardIcon(QStyle::SP_ArrowUp))),
        QLineEdit::TrailingPosition);
    togglePw->setCheckable(true);
    togglePw->setText("Mostrar contrase\u00f1a");
    togglePw->setToolTip("Alternar visibilidad de la contrase\u00f1a");
    connect(togglePw, &QAction::triggered, this, &RegisterWindow::togglePasswordVisibility);

    layout->addWidget(passwordEdit);
    passwordLabel->setBuddy(passwordEdit);

    layout->addSpacing(16);

    QLabel *confirmLabel = new QLabel("Confirmar contrase\u00f1a");
    confirmLabel->setStyleSheet("font-size: 12px; font-weight: 500; color: #6e6e73; "
                                "letter-spacing: 0.3px; margin-bottom: 4px;");
    layout->addWidget(confirmLabel);

    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("Repita la contrase\u00f1a");
    confirmPasswordEdit->setAccessibleName("Confirmar contrase\u00f1a");

    QAction *toggleConfirm = confirmPasswordEdit->addAction(
        QIcon::fromTheme("view-visible",
                         QIcon::fromTheme("dialog-password",
                                          QApplication::style()->standardIcon(QStyle::SP_ArrowUp))),
        QLineEdit::TrailingPosition);
    toggleConfirm->setCheckable(true);
    toggleConfirm->setText("Mostrar contrase\u00f1a");
    toggleConfirm->setToolTip("Alternar visibilidad de la contrase\u00f1a");
    connect(toggleConfirm, &QAction::triggered, this, &RegisterWindow::toggleConfirmVisibility);
    layout->addWidget(confirmPasswordEdit);
    confirmLabel->setBuddy(confirmPasswordEdit);

    layout->addSpacing(24);

    registerButton = new QPushButton("Registrarse");
    registerButton->setObjectName("btnPrimary");
    registerButton->setCursor(Qt::PointingHandCursor);
    registerButton->setMinimumHeight(42);
    layout->addWidget(registerButton);

    layout->addStretch();

    connect(registerButton, &QPushButton::clicked, this, [this]() {
        AuthController auth;
        auto result = auth.registerUser(
            usernameEdit->text(), passwordEdit->text(),
            confirmPasswordEdit->text());
        if (result.success) {
            m_registeredUsername = usernameEdit->text();
            QMessageBox::information(this, "Exito",
                                     "Usuario registrado correctamente");
            accept();
        } else {
            QMessageBox::warning(this, "Error", result.errorMessage);
        }
    });
}

void RegisterWindow::togglePasswordVisibility() {
    bool hidden = passwordEdit->echoMode() == QLineEdit::Password;
    passwordEdit->setEchoMode(
        hidden ? QLineEdit::Normal : QLineEdit::Password);
    QAction *a = passwordEdit->actions().value(0);
    if (a) {
        a->setChecked(!hidden);
        a->setText(hidden ? "Ocultar contrase\u00f1a" : "Mostrar contrase\u00f1a");
    }
}

void RegisterWindow::toggleConfirmVisibility() {
    bool hidden = confirmPasswordEdit->echoMode() == QLineEdit::Password;
    confirmPasswordEdit->setEchoMode(
        hidden ? QLineEdit::Normal : QLineEdit::Password);
    QAction *a = confirmPasswordEdit->actions().value(0);
    if (a) {
        a->setChecked(!hidden);
        a->setText(hidden ? "Ocultar contrase\u00f1a" : "Mostrar contrase\u00f1a");
    }
}
