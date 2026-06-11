#!/usr/bin/env bash
set -euo pipefail

MARIADB=$(command -v mariadb 2>/dev/null || command -v mysql 2>/dev/null)
if [ -z "$MARIADB" ]; then
    echo "Error: MariaDB/MySQL CLI no encontrado"
    exit 1
fi

if ! pgrep -x mariadbd >/dev/null 2>&1 && ! pgrep -x mysqld >/dev/null 2>&1; then
    echo "MariaDB no esta corriendo. Inicialo con:"
    echo "  sudo systemctl start mariadb"
    exit 1
fi

if $MARIADB -u facturador -p'facturador123' -e "SELECT 1;" >/dev/null 2>&1; then
    echo "Usuario 'facturador' ya existe, solo creo schema si falta..."
    $MARIADB -u facturador -p'facturador123' <<-SQL
        CREATE DATABASE IF NOT EXISTS facturador DEFAULT CHARACTER SET utf8mb4;
        GRANT ALL PRIVILEGES ON facturador.* TO 'facturador'@'localhost';
        FLUSH PRIVILEGES;
SQL
    echo "OK"
    exit 0
fi

echo "Creando usuario 'facturador' y base de datos..."
echo "Se necesita acceso de administrador a MariaDB."

if [ "$(id -u)" -eq 0 ]; then
    $MARIADB <<-SQL
        CREATE USER IF NOT EXISTS 'facturador'@'localhost'
        IDENTIFIED BY 'facturador123';
        CREATE DATABASE IF NOT EXISTS facturador
        DEFAULT CHARACTER SET utf8mb4;
        GRANT ALL PRIVILEGES ON facturador.* TO 'facturador'@'localhost';
        FLUSH PRIVILEGES;
SQL
elif command -v sudo >/dev/null; then
    sudo $MARIADB <<-SQL
        CREATE USER IF NOT EXISTS 'facturador'@'localhost'
        IDENTIFIED BY 'facturador123';
        CREATE DATABASE IF NOT EXISTS facturador
        DEFAULT CHARACTER SET utf8mb4;
        GRANT ALL PRIVILEGES ON facturador.* TO 'facturador'@'localhost';
        FLUSH PRIVILEGES;
SQL
else
    echo "No se pudo ejecutar como root. Corre manualmente:"
    echo
    echo "  sudo $MARIADB"
    echo
    echo "Y pega esto:"
    cat <<-SQL
        CREATE USER IF NOT EXISTS 'facturador'@'localhost'
        IDENTIFIED BY 'facturador123';
        CREATE DATABASE IF NOT EXISTS facturador
        DEFAULT CHARACTER SET utf8mb4;
        GRANT ALL PRIVILEGES ON facturador.* TO 'facturador'@'localhost';
        FLUSH PRIVILEGES;
SQL
    exit 1
fi

echo
echo "Setup completo. Ya podes ejecutar el facturador."
