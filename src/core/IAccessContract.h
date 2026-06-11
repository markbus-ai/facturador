#pragma once

class IAccessContract {
public:
    virtual ~IAccessContract() = default;
    virtual bool canManageProducts() const = 0;
    virtual bool canViewAllInvoices() const = 0;
    virtual bool canManageUsers() const = 0;
    virtual bool canManageClients() const = 0;
    virtual bool canGenerateReports() const = 0;
    virtual bool canCreateInvoice() const = 0;
    virtual bool canQueryProducts() const = 0;
};

class AdminAccess : public IAccessContract {
public:
    bool canManageProducts() const override { return true; }
    bool canViewAllInvoices() const override { return true; }
    bool canManageUsers() const override { return true; }
    bool canManageClients() const override { return true; }
    bool canGenerateReports() const override { return true; }
    bool canCreateInvoice() const override { return true; }
    bool canQueryProducts() const override { return true; }
};

class UserAccess : public IAccessContract {
public:
    bool canManageProducts() const override { return false; }
    bool canViewAllInvoices() const override { return false; }
    bool canManageUsers() const override { return false; }
    bool canManageClients() const override { return false; }
    bool canGenerateReports() const override { return false; }
    bool canCreateInvoice() const override { return true; }
    bool canQueryProducts() const override { return true; }
};

class RepositorAccess : public IAccessContract {
public:
    bool canManageProducts() const override { return true; }
    bool canViewAllInvoices() const override { return false; }
    bool canManageUsers() const override { return false; }
    bool canManageClients() const override { return false; }
    bool canGenerateReports() const override { return true; }
    bool canCreateInvoice() const override { return false; }
    bool canQueryProducts() const override { return true; }
};
