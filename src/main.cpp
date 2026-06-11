#include "ui/login.h"
#include "ui/userwindow.h"
#include "ui/adminwindow.h"
#include "ui/repositorwindow.h"
#include "database/DatabaseManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <functional>

static const char *STYLESHEET = R"(
    QMainWindow, QDialog {
        background: #f5f5f7;
    }
    QWidget {
        font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif;
        font-size: 13px;
        color: #1d1d1f;
    }
    QGroupBox {
        background: #ffffff;
        border: 1px solid #e5e5ea;
        border-radius: 6px;
        margin-top: 14px;
        padding: 16px 16px 16px 16px;
        font-weight: 600;
        font-size: 12px;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        subcontrol-position: top left;
        padding: 0 8px;
        color: #535357;
        letter-spacing: 0.5px;
    }
    QHeaderView {
        background: #f5f5f7;
    }
    QTableWidget {
        background: #ffffff;
        border: 1px solid #e5e5ea;
        border-radius: 6px;
        gridline-color: #e8e8ed;
        outline: none;
        selection-background-color: #e8f0fe;
        selection-color: #1d1d1f;
        alternate-background-color: #fafafc;
    }
    QTableWidget::item {
        padding: 6px 10px;
        color: #1d1d1f;
    }
    QTableWidget::item:selected {
        background: #e8f0fe;
        color: #1d1d1f;
    }
    QTableWidget::item:alternate {
        background: #fafafc;
    }
    QHeaderView::section {
        background: #f5f5f7;
        border: none;
        border-bottom: 1px solid #e5e5ea;
        border-right: 1px solid #e8e8ed;
        padding: 10px 10px;
        font-weight: 600;
        font-size: 12px;
        color: #535357;
    }
    QHeaderView::section:checked {
        background: #eeeff1;
    }
    QHeaderView::section:hover {
        background: #eaeaf0;
    }
    QPushButton {
        background: #ffffff;
        border: 1px solid #d2d2d7;
        border-radius: 6px;
        padding: 8px 18px;
        color: #1d1d1f;
        font-size: 13px;
        font-weight: 500;
    }
    QPushButton:hover {
        background: #f5f5f7;
        border-color: #c7c7cc;
    }
    QPushButton:pressed {
        background: #e8e8ed;
    }
    QPushButton#btnPrimary {
        background: #1d1d1f;
        color: #ffffff;
        border: none;
        font-weight: 500;
    }
    QPushButton#btnPrimary:hover {
        background: #2d2d2f;
    }
    QPushButton#btnPrimary:pressed {
        background: #000000;
    }
    QPushButton#btnDanger {
        color: #d92b20;
        border-color: #ffd1cf;
    }
    QPushButton#btnDanger:hover {
        background: #fff5f5;
        border-color: #d92b20;
    }
    QPushButton#btnText {
        background: transparent;
        border: none;
        color: #0055cc;
        padding: 6px;
        font-weight: 400;
    }
    QPushButton#btnText:hover {
        color: #003d99;
        text-decoration: underline;
    }
    QLineEdit {
        border: 1px solid #d2d2d7;
        border-radius: 6px;
        padding: 10px 12px;
        background: #ffffff;
        color: #1d1d1f;
        font-size: 13px;
    }
    QLineEdit:focus {
        border-color: #007aff;
    }
    QComboBox {
        border: 1px solid #d2d2d7;
        border-radius: 6px;
        padding: 8px 12px;
        background: #ffffff;
        color: #1d1d1f;
    }
    QComboBox:focus {
        border-color: #007aff;
    }
    QComboBox::drop-down {
        border: none;
        width: 28px;
    }
    QDoubleSpinBox, QSpinBox {
        border: 1px solid #d2d2d7;
        border-radius: 6px;
        padding: 6px 28px 6px 10px;
        background: #ffffff;
        color: #1d1d1f;
    }
    QDoubleSpinBox:focus, QSpinBox:focus {
        border-color: #007aff;
    }
    QDoubleSpinBox::up-button, QSpinBox::up-button {
        border: none;
        background: transparent;
        width: 20px;
        height: 12px;
        subcontrol-position: top right;
        subcontrol-origin: padding;
    }
    QDoubleSpinBox::down-button, QSpinBox::down-button {
        border: none;
        background: transparent;
        width: 20px;
        height: 12px;
        subcontrol-position: bottom right;
        subcontrol-origin: padding;
    }
    QDoubleSpinBox::up-arrow, QSpinBox::up-arrow {
        width: 8px;
        height: 8px;
    }
    QDoubleSpinBox::down-arrow, QSpinBox::down-arrow {
        width: 8px;
        height: 8px;
    }
    QTabWidget::pane {
        border: none;
        background: transparent;
    }
    QTabBar::tab {
        padding: 10px 18px;
        border: none;
        border-bottom: 2px solid transparent;
        color: #6e6e73;
        font-weight: 500;
        font-size: 13px;
    }
    QTabBar::tab:selected {
        color: #1d1d1f;
        border-bottom: 2px solid #1d1d1f;
    }
    QTabBar::tab:hover:!selected {
        color: #535357;
    }
    QLabel {
        color: #1d1d1f;
    }
    QStatusBar {
        background: #fafafc;
        border-top: 1px solid #e5e5ea;
        font-size: 12px;
        color: #6e6e73;
    }
    QScrollBar:vertical {
        background: transparent;
        width: 8px;
    }
    QScrollBar::handle:vertical {
        background: #c7c7cc;
        border-radius: 4px;
        min-height: 30px;
    }
    QScrollBar::handle:vertical:hover {
        background: #a8a8ad;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0;
    }
)";

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(STYLESHEET);

    QFont defaultFont = app.font();
    defaultFont.setPointSize(13);
    defaultFont.setFamily("Segoe UI");
    app.setFont(defaultFont);

    if (!DatabaseManager::instance().isConnected()) {
        QMessageBox::critical(nullptr, "Error de conexion",
            "No se pudo conectar a MariaDB/MySQL.\n\n"
            "Ejecuta el script de setup:\n"
            "  ./setup.sh\n\n"
            "Si el problema persiste, revisa:\n"
            "  sudo systemctl status mariadb\n"
            "  journalctl -xeu mariadb");
        return 1;
    }

    std::function<void()> startSession;
    startSession = [&app, &startSession]() {
        LoginWindow login;
        if (login.exec() != QDialog::Accepted) {
            app.quit();
            return;
        }

        int userId = login.userId();
        const QString &role = login.role();
        const QString &username = login.username();

        QWidget *w = nullptr;
        if (role == "admin")
            w = new AdminWindow(username, userId);
        else if (role == "repositor")
            w = new RepositorWindow(username, userId);
        else
            w = new UserWindow(username, userId);

        w->setAttribute(Qt::WA_DeleteOnClose);

        auto onLogout = [&app, &startSession, w]() {
            app.setQuitOnLastWindowClosed(false);
            w->close();
            QTimer::singleShot(0, [&app, &startSession]() {
                app.setQuitOnLastWindowClosed(true);
                startSession();
            });
        };

        if (auto *admin = qobject_cast<AdminWindow *>(w))
            QObject::connect(admin, &AdminWindow::loggedOut, onLogout);
        else if (auto *repo = qobject_cast<RepositorWindow *>(w))
            QObject::connect(repo, &RepositorWindow::loggedOut, onLogout);
        else if (auto *user = qobject_cast<UserWindow *>(w))
            QObject::connect(user, &UserWindow::loggedOut, onLogout);

        w->show();
    };

    startSession();
    return app.exec();
}
