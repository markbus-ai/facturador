cmake_minimum_required(VERSION 3.16)
project(facturador VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(facturador
    src/main.cpp
    src/core/IDiscountStrategy.cpp
    src/services/InvoiceCalculator.cpp
    src/services/InvoiceService.cpp
    src/database/DatabaseManager.cpp
    src/database/UserRepository.cpp
    src/database/ProductRepository.cpp
    src/database/ClientRepository.cpp
    src/database/InvoiceRepository.cpp
    src/database/SupplierRepository.cpp
    src/controllers/AuthController.cpp
    src/controllers/InventoryController.cpp
    src/controllers/BillingController.cpp
    src/controllers/ClientController.cpp
    src/controllers/SupplierController.cpp
    src/ui/login.cpp
    src/ui/registerwindows.cpp
    src/ui/adminwindow.cpp
    src/ui/userwindow.cpp
    src/ui/invoicedialog.cpp
    src/ui/productdialog.cpp
    src/ui/clientdialog.cpp
    src/ui/supplierdialog.cpp
    src/ui/repositorwindow.cpp
)

target_include_directories(facturador PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(facturador PRIVATE Qt6::Widgets Qt6::Sql)
