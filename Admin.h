#pragma once
#include "User.h"
#include "Database.h"
#include <string>

class Admin : public User {
private:
    Database* db;

    std::string escapeCSV(const std::string& str);

    // Member Operations
    void viewAllMembers();
    void addNewMember();
    void updateMember();
    void deleteMember();
    void searchMember();
    void viewMemberDetails(int memberId);

    // Revenue Analytics Methods
    void revenueOverview();
    void monthlyRevenueReport();
    void paymentMethodAnalysis();
    void topMembersBySpending();
    void exportRevenueReport();

    // CSV Export
    void exportExpiringToCSV(int daysThreshold);

    // Main Analytics Methods
    void viewRevenueAnalytics();
    void checkExpiringMemberships();

    // Payment Operations
    void viewPayments();
    void addPayment();
    void deletePayment();
    void managePayments();

    // Attendance Operations
    void viewAttendance();

    // Membership Types Operations
    void manageMembershipTypes();
    void viewMembershipTypes();
    void addMembershipType();
    void updateMembershipType();
    void deleteMembershipType();

    // Admin Approval Methods
    void manageAdminApprovals();
    void viewPendingAdmins();
    void approveAdmin(int userId);
    void rejectAdmin(int userId);
    void viewApprovedAdmins();

    // Helper methods for menu organization
    void manageMembers();  // Added

public:
    Admin();
    Admin(int id, const std::string& name, const std::string& email, const std::string& password);
    void displayMenu() override;

    // Helper method to get user ID (inherited from User, but can be overridden if needed)
    int getUserId() const { return User::getUserId(); }
};