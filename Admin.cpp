#include "Admin.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <ctime>
#include <limits>

Admin::Admin() : User() {
    db = Database::getInstance();
}

Admin::Admin(int id, const std::string& name, const std::string& email, const std::string& password)
    : User(id, name, email, password), db(nullptr) {
    db = Database::getInstance();
}

void Admin::displayMenu() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "           ADMIN DASHBOARD\n";
        std::cout << "========================================\n";
        std::cout << "1. Manage Members\n";
        std::cout << "2. View Revenue Analytics\n";
        std::cout << "3. Check Expiring Memberships\n";
        std::cout << "4. Manage Payments\n";
        std::cout << "5. View Attendance\n";
        std::cout << "6. Manage Membership Types\n";
        std::cout << "7. Manage Admin Approvals\n";
        std::cout << "8. Logout\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: manageMembers(); break;
        case 2: viewRevenueAnalytics(); break;
        case 3: checkExpiringMemberships(); break;
        case 4: managePayments(); break;
        case 5: viewAttendance(); break;
        case 6: manageMembershipTypes(); break;
        case 7: manageAdminApprovals(); break;
        case 8: std::cout << "Logging out...\n"; break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 8);
}

// ============================================
// MEMBER MANAGEMENT
// ============================================
void Admin::manageMembers() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "         MANAGE MEMBERS\n";
        std::cout << "========================================\n";
        std::cout << "1. View All Members\n";
        std::cout << "2. Add New Member\n";
        std::cout << "3. Update Member\n";
        std::cout << "4. Delete Member\n";
        std::cout << "5. Search Member\n";
        std::cout << "6. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: viewAllMembers(); break;
        case 2: addNewMember(); break;
        case 3: updateMember(); break;
        case 4: deleteMember(); break;
        case 5: searchMember(); break;
        case 6: std::cout << "Returning to main menu...\n"; break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 6);
}

void Admin::viewAllMembers() {
    std::cout << "\n========================================\n";
    std::cout << "         ALL MEMBERS LIST\n";
    std::cout << "========================================\n";

    std::string query = "SELECT m.member_id, m.first_name, m.last_name, m.email, m.phone, "
        "m.payment_status, m.join_date, t.type_name "
        "FROM members m "
        "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
        "ORDER BY m.member_id";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(5) << "ID"
            << std::setw(20) << "NAME"
            << std::setw(25) << "EMAIL"
            << std::setw(15) << "PHONE"
            << std::setw(12) << "STATUS"
            << std::setw(15) << "PLAN" << "\n";
        std::cout << std::string(92, '-') << "\n";

        int count = 0;
        while (res->next()) {
            count++;
            std::string fullName = res->getString("first_name") + " " + res->getString("last_name");
            std::string plan = res->isNull("type_name") ? "None" : res->getString("type_name");

            std::cout << std::left
                << std::setw(5) << res->getInt("member_id")
                << std::setw(20) << fullName.substr(0, 18)
                << std::setw(25) << res->getString("email").substr(0, 23)
                << std::setw(15) << res->getString("phone")
                << std::setw(12) << res->getString("payment_status")
                << std::setw(15) << plan << "\n";
        }

        std::cout << std::string(92, '-') << "\n";
        std::cout << "Total Members: " << count << "\n";
        delete res;
    }
    else {
        std::cout << "No members found or error loading data.\n";
    }
}

void Admin::addNewMember() {
    std::cout << "\n========================================\n";
    std::cout << "         ADD NEW MEMBER\n";
    std::cout << "========================================\n";

    std::string firstName, lastName, email, phone, password;

    std::cin.ignore();
    std::cout << "First Name: ";
    std::getline(std::cin, firstName);

    std::cout << "Last Name: ";
    std::getline(std::cin, lastName);

    std::cout << "Email: ";
    std::getline(std::cin, email);

    std::cout << "Phone: ";
    std::getline(std::cin, phone);

    std::cout << "Password: ";
    std::getline(std::cin, password);

    // Show membership types
    std::cout << "\nAvailable Membership Plans:\n";
    std::string typeQuery = "SELECT type_id, type_name, price FROM membership_types";
    sql::ResultSet* typeRes = db->executeQuery(typeQuery);

    if (typeRes) {
        std::cout << "0 - No membership\n";
        while (typeRes->next()) {
            std::cout << typeRes->getInt("type_id") << " - "
                << typeRes->getString("type_name") << " (RM"
                << std::fixed << std::setprecision(2) << typeRes->getDouble("price") << ")\n";
        }
        delete typeRes;
    }

    std::cout << "Select Membership Type ID: ";
    int membershipTypeId;
    std::cin >> membershipTypeId;

    // Insert into users table first
    std::string fullName = firstName + " " + lastName;
    std::string userQuery = "INSERT INTO users (name, email, password, phone, role) VALUES ('" +
        fullName + "', '" + email + "', '" + password + "', '" + phone + "', 'member')";

    if (!db->executeUpdate(userQuery)) {
        std::cout << "Failed to create user account!\n";
        return;
    }

    // Get new user ID
    std::string getUserIdQuery = "SELECT LAST_INSERT_ID() as new_user_id";
    sql::ResultSet* userIdRes = db->executeQuery(getUserIdQuery);
    int newUserId = 0;

    if (userIdRes && userIdRes->next()) {
        newUserId = userIdRes->getInt("new_user_id");
    }
    if (userIdRes) delete userIdRes;

    // Insert into members table
    std::string memberQuery = "INSERT INTO members (user_id, first_name, last_name, phone, email, "
        "membership_type_id, join_date, payment_status) VALUES (" +
        std::to_string(newUserId) + ", '" + firstName + "', '" + lastName + "', '" +
        phone + "', '" + email + "', " +
        (membershipTypeId > 0 ? std::to_string(membershipTypeId) : "NULL") +
        ", CURDATE(), 'pending')";

    if (db->executeUpdate(memberQuery)) {
        std::cout << "\nMember added successfully!\n";
    }
    else {
        std::cout << "\nFailed to add member!\n";
    }
}

void Admin::updateMember() {
    std::cout << "\n========================================\n";
    std::cout << "         UPDATE MEMBER\n";
    std::cout << "========================================\n";

    viewAllMembers();

    std::cout << "\nEnter Member ID to update (0 to cancel): ";
    int memberId;
    std::cin >> memberId;

    if (memberId == 0) return;

    // Check if member exists
    std::string checkQuery = "SELECT * FROM members WHERE member_id = " + std::to_string(memberId);
    sql::ResultSet* checkRes = db->executeQuery(checkQuery);

    if (!checkRes || !checkRes->next()) {
        std::cout << "Member not found!\n";
        if (checkRes) delete checkRes;
        return;
    }

    std::cout << "\nCurrent Details:\n";
    std::cout << "Name: " << checkRes->getString("first_name") << " " << checkRes->getString("last_name") << "\n";
    std::cout << "Phone: " << checkRes->getString("phone") << "\n";
    std::cout << "Email: " << checkRes->getString("email") << "\n";
    delete checkRes;

    std::cout << "\nWhat would you like to update?\n";
    std::cout << "1. Phone Number\n";
    std::cout << "2. Email\n";
    std::cout << "3. Membership Type\n";
    std::cout << "4. Payment Status\n";
    std::cout << "5. Cancel\n";
    std::cout << "Enter choice: ";

    int updateChoice;
    std::cin >> updateChoice;
    std::cin.ignore();

    std::string updateQuery;

    switch (updateChoice) {
    case 1: {
        std::cout << "New Phone: ";
        std::string newPhone;
        std::getline(std::cin, newPhone);
        updateQuery = "UPDATE members SET phone = '" + newPhone + "' WHERE member_id = " + std::to_string(memberId);
        break;
    }
    case 2: {
        std::cout << "New Email: ";
        std::string newEmail;
        std::getline(std::cin, newEmail);
        updateQuery = "UPDATE members SET email = '" + newEmail + "' WHERE member_id = " + std::to_string(memberId);
        break;
    }
    case 3: {
        std::cout << "\nAvailable Membership Plans:\n";
        std::string typeQuery = "SELECT type_id, type_name, price FROM membership_types";
        sql::ResultSet* typeRes = db->executeQuery(typeQuery);
        if (typeRes) {
            while (typeRes->next()) {
                std::cout << typeRes->getInt("type_id") << " - " << typeRes->getString("type_name") << "\n";
            }
            delete typeRes;
        }
        std::cout << "New Membership Type ID: ";
        int newTypeId;
        std::cin >> newTypeId;
        updateQuery = "UPDATE members SET membership_type_id = " + std::to_string(newTypeId) +
            " WHERE member_id = " + std::to_string(memberId);
        break;
    }
    case 4: {
        std::cout << "New Status (paid/pending/expired): ";
        std::string newStatus;
        std::getline(std::cin, newStatus);
        updateQuery = "UPDATE members SET payment_status = '" + newStatus + "' WHERE member_id = " + std::to_string(memberId);
        break;
    }
    default:
        std::cout << "Update cancelled.\n";
        return;
    }

    if (db->executeUpdate(updateQuery)) {
        std::cout << "Member updated successfully!\n";
    }
    else {
        std::cout << "Failed to update member!\n";
    }
}

void Admin::deleteMember() {
    std::cout << "\n========================================\n";
    std::cout << "         DELETE MEMBER\n";
    std::cout << "========================================\n";

    viewAllMembers();

    std::cout << "\nEnter Member ID to delete (0 to cancel): ";
    int memberId;
    std::cin >> memberId;

    if (memberId == 0) return;

    // Get user_id before deleting
    std::string getUserQuery = "SELECT user_id, first_name, last_name FROM members WHERE member_id = " + std::to_string(memberId);
    sql::ResultSet* userRes = db->executeQuery(getUserQuery);

    if (!userRes || !userRes->next()) {
        std::cout << "Member not found!\n";
        if (userRes) delete userRes;
        return;
    }

    int userId = userRes->getInt("user_id");
    std::string memberName = userRes->getString("first_name") + " " + userRes->getString("last_name");
    delete userRes;

    std::cout << "Are you sure you want to delete " << memberName << "? (1=Yes, 0=No): ";
    int confirm;
    std::cin >> confirm;

    if (confirm != 1) {
        std::cout << "Deletion cancelled.\n";
        return;
    }

    // Delete from members table (cascade should handle related records)
    std::string deleteQuery = "DELETE FROM members WHERE member_id = " + std::to_string(memberId);

    if (db->executeUpdate(deleteQuery)) {
        // Also delete from users table
        std::string deleteUserQuery = "DELETE FROM users WHERE user_id = " + std::to_string(userId);
        db->executeUpdate(deleteUserQuery);
        std::cout << "Member deleted successfully!\n";
    }
    else {
        std::cout << "Failed to delete member!\n";
    }
}

void Admin::searchMember() {
    std::cout << "\n========================================\n";
    std::cout << "         SEARCH MEMBER\n";
    std::cout << "========================================\n";

    std::cout << "Search by:\n";
    std::cout << "1. Name\n";
    std::cout << "2. Email\n";
    std::cout << "3. Phone\n";
    std::cout << "4. Member ID\n";
    std::cout << "Enter choice: ";

    int searchChoice;
    std::cin >> searchChoice;
    std::cin.ignore();

    std::string searchQuery;
    std::string searchTerm;

    switch (searchChoice) {
    case 1:
        std::cout << "Enter name to search: ";
        std::getline(std::cin, searchTerm);
        searchQuery = "SELECT m.*, t.type_name FROM members m "
            "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
            "WHERE CONCAT(m.first_name, ' ', m.last_name) LIKE '%" + searchTerm + "%'";
        break;
    case 2:
        std::cout << "Enter email to search: ";
        std::getline(std::cin, searchTerm);
        searchQuery = "SELECT m.*, t.type_name FROM members m "
            "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
            "WHERE m.email LIKE '%" + searchTerm + "%'";
        break;
    case 3:
        std::cout << "Enter phone to search: ";
        std::getline(std::cin, searchTerm);
        searchQuery = "SELECT m.*, t.type_name FROM members m "
            "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
            "WHERE m.phone LIKE '%" + searchTerm + "%'";
        break;
    case 4:
        std::cout << "Enter Member ID: ";
        int searchId;
        std::cin >> searchId;
        searchQuery = "SELECT m.*, t.type_name FROM members m "
            "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
            "WHERE m.member_id = " + std::to_string(searchId);
        break;
    default:
        std::cout << "Invalid choice!\n";
        return;
    }

    sql::ResultSet* res = db->executeQuery(searchQuery);

    if (res) {
        std::cout << "\n=== SEARCH RESULTS ===\n";
        std::cout << std::left
            << std::setw(5) << "ID"
            << std::setw(20) << "NAME"
            << std::setw(25) << "EMAIL"
            << std::setw(15) << "PHONE"
            << std::setw(12) << "STATUS" << "\n";
        std::cout << std::string(77, '-') << "\n";

        int count = 0;
        while (res->next()) {
            count++;
            std::string fullName = res->getString("first_name") + " " + res->getString("last_name");

            std::cout << std::left
                << std::setw(5) << res->getInt("member_id")
                << std::setw(20) << fullName.substr(0, 18)
                << std::setw(25) << res->getString("email").substr(0, 23)
                << std::setw(15) << res->getString("phone")
                << std::setw(12) << res->getString("payment_status") << "\n";
        }

        if (count == 0) {
            std::cout << "No members found matching your search.\n";
        }
        else {
            std::cout << std::string(77, '-') << "\n";
            std::cout << "Found " << count << " member(s)\n";

            std::cout << "\nView detailed info? Enter Member ID (0 to skip): ";
            int detailId;
            std::cin >> detailId;
            if (detailId > 0) {
                viewMemberDetails(detailId);
            }
        }
        delete res;
    }
}

void Admin::viewMemberDetails(int memberId) {
    std::string query = "SELECT m.*, t.type_name, t.price, t.duration_months "
        "FROM members m "
        "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
        "WHERE m.member_id = " + std::to_string(memberId);

    sql::ResultSet* res = db->executeQuery(query);

    if (res && res->next()) {
        std::cout << "\n========================================\n";
        std::cout << "         MEMBER DETAILS\n";
        std::cout << "========================================\n";
        std::cout << "Member ID:    " << res->getInt("member_id") << "\n";
        std::cout << "Name:         " << res->getString("first_name") << " " << res->getString("last_name") << "\n";
        std::cout << "Email:        " << res->getString("email") << "\n";
        std::cout << "Phone:        " << res->getString("phone") << "\n";
        std::cout << "Join Date:    " << res->getString("join_date") << "\n";
        std::cout << "Status:       " << res->getString("payment_status") << "\n";

        if (!res->isNull("type_name")) {
            std::cout << "Plan:         " << res->getString("type_name") << "\n";
            std::cout << "Plan Price:   RM" << std::fixed << std::setprecision(2) << res->getDouble("price") << "\n";
        }

        if (!res->isNull("start_date")) {
            std::cout << "Start Date:   " << res->getString("start_date") << "\n";
        }
        if (!res->isNull("expiry_date")) {
            std::cout << "Expiry Date:  " << res->getString("expiry_date") << "\n";
        }
        std::cout << "========================================\n";
    }
    else {
        std::cout << "Member not found!\n";
    }

    if (res) delete res;
}

// ============================================
// REVENUE ANALYTICS
// ============================================
void Admin::viewRevenueAnalytics() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "         REVENUE ANALYTICS\n";
        std::cout << "========================================\n";
        std::cout << "1. Revenue Overview\n";
        std::cout << "2. Monthly Revenue Report\n";
        std::cout << "3. Payment Method Analysis\n";
        std::cout << "4. Top Members by Spending\n";
        std::cout << "5. Export Full Report to TXT\n";
        std::cout << "6. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: revenueOverview(); break;
        case 2: monthlyRevenueReport(); break;
        case 3: paymentMethodAnalysis(); break;
        case 4: topMembersBySpending(); break;
        case 5: exportRevenueReport(); break;
        case 6: break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 6);
}

void Admin::revenueOverview() {
    std::cout << "\n========================================\n";
    std::cout << "         REVENUE OVERVIEW\n";
    std::cout << "========================================\n";

    // Total revenue
    std::string totalQuery = "SELECT SUM(amount) as total FROM payments";
    sql::ResultSet* totalRes = db->executeQuery(totalQuery);
    if (totalRes && totalRes->next()) {
        double total = totalRes->isNull("total") ? 0.0 : totalRes->getDouble("total");
        std::cout << "Total Revenue:        RM" << std::fixed << std::setprecision(2) << total << "\n";
    }
    if (totalRes) delete totalRes;

    // This month's revenue
    std::string monthQuery = "SELECT SUM(amount) as monthly FROM payments "
        "WHERE MONTH(payment_date) = MONTH(CURDATE()) AND YEAR(payment_date) = YEAR(CURDATE())";
    sql::ResultSet* monthRes = db->executeQuery(monthQuery);
    if (monthRes && monthRes->next()) {
        double monthly = monthRes->isNull("monthly") ? 0.0 : monthRes->getDouble("monthly");
        std::cout << "This Month's Revenue: RM" << std::fixed << std::setprecision(2) << monthly << "\n";
    }
    if (monthRes) delete monthRes;

    // Today's revenue
    std::string todayQuery = "SELECT SUM(amount) as today FROM payments WHERE payment_date = CURDATE()";
    sql::ResultSet* todayRes = db->executeQuery(todayQuery);
    if (todayRes && todayRes->next()) {
        double today = todayRes->isNull("today") ? 0.0 : todayRes->getDouble("today");
        std::cout << "Today's Revenue:      RM" << std::fixed << std::setprecision(2) << today << "\n";
    }
    if (todayRes) delete todayRes;

    // Total transactions
    std::string countQuery = "SELECT COUNT(*) as count FROM payments";
    sql::ResultSet* countRes = db->executeQuery(countQuery);
    if (countRes && countRes->next()) {
        std::cout << "Total Transactions:   " << countRes->getInt("count") << "\n";
    }
    if (countRes) delete countRes;

    // Active members
    std::string activeQuery = "SELECT COUNT(*) as active FROM members WHERE payment_status = 'paid'";
    sql::ResultSet* activeRes = db->executeQuery(activeQuery);
    if (activeRes && activeRes->next()) {
        std::cout << "Active Members:       " << activeRes->getInt("active") << "\n";
    }
    if (activeRes) delete activeRes;

    std::cout << "========================================\n";
}

void Admin::monthlyRevenueReport() {
    std::cout << "\n========================================\n";
    std::cout << "      MONTHLY REVENUE REPORT\n";
    std::cout << "========================================\n";

    // Group by month and sum
    std::string query = "SELECT DATE_FORMAT(payment_date, '%Y-%m') as month, "
        "SUM(amount) as total, COUNT(*) as transactions "
        "FROM payments "
        "GROUP BY DATE_FORMAT(payment_date, '%Y-%m') "
        "ORDER BY month DESC "
        "LIMIT 12";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(15) << "MONTH"
            << std::setw(15) << "REVENUE"
            << std::setw(15) << "TRANSACTIONS" << "\n";
        std::cout << std::string(45, '-') << "\n";

        while (res->next()) {
            std::cout << std::left
                << std::setw(15) << res->getString("month")
                << "RM" << std::setw(14) << std::fixed << std::setprecision(2) << res->getDouble("total")
                << std::setw(15) << res->getInt("transactions") << "\n";
        }
        std::cout << std::string(45, '-') << "\n";
        delete res;
    }
}

void Admin::paymentMethodAnalysis() {
    std::cout << "\n========================================\n";
    std::cout << "     PAYMENT METHOD ANALYSIS\n";
    std::cout << "========================================\n";

    std::string query = "SELECT payment_method, COUNT(*) as count, SUM(amount) as total "
        "FROM payments "
        "GROUP BY payment_method "
        "ORDER BY total DESC";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(20) << "METHOD"
            << std::setw(15) << "TRANSACTIONS"
            << std::setw(15) << "TOTAL" << "\n";
        std::cout << std::string(50, '-') << "\n";

        while (res->next()) {
            std::cout << std::left
                << std::setw(20) << res->getString("payment_method")
                << std::setw(15) << res->getInt("count")
                << "RM" << std::fixed << std::setprecision(2) << res->getDouble("total") << "\n";
        }
        std::cout << std::string(50, '-') << "\n";
        delete res;
    }
}

void Admin::topMembersBySpending() {
    std::cout << "\n========================================\n";
    std::cout << "      TOP MEMBERS BY SPENDING\n";
    std::cout << "========================================\n";

    std::string query = "SELECT m.member_id, m.first_name, m.last_name, "
        "SUM(p.amount) as total_spent, COUNT(p.payment_id) as payments "
        "FROM members m "
        "JOIN payments p ON m.member_id = p.member_id "
        "GROUP BY m.member_id, m.first_name, m.last_name "
        "ORDER BY total_spent DESC "
        "LIMIT 10";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(5) << "RANK"
            << std::setw(25) << "NAME"
            << std::setw(15) << "TOTAL SPENT"
            << std::setw(12) << "PAYMENTS" << "\n";
        std::cout << std::string(57, '-') << "\n";

        int rank = 1;
        while (res->next()) {
            std::string fullName = res->getString("first_name") + " " + res->getString("last_name");
            std::cout << std::left
                << std::setw(5) << rank++
                << std::setw(25) << fullName.substr(0, 23)
                << "RM" << std::setw(14) << std::fixed << std::setprecision(2) << res->getDouble("total_spent")
                << std::setw(12) << res->getInt("payments") << "\n";
        }
        std::cout << std::string(57, '-') << "\n";
        delete res;
    }
}

void Admin::exportRevenueReport() {
    // Get current date/time for filename
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char dateStr[20];
    strftime(dateStr, sizeof(dateStr), "%Y%m%d_%H%M%S", &ltm);

    std::string filename = "revenue_report_" + std::string(dateStr) + ".txt";
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cout << "Failed to create report file!\n";
        return;
    }

    std::cout << "Generating report...\n";

    // Header
    file << "================================================================\n";
    file << "              GYM MANAGEMENT SYSTEM - REVENUE REPORT\n";
    file << "================================================================\n";
    char reportDate[50];
    strftime(reportDate, sizeof(reportDate), "Generated: %Y-%m-%d %H:%M:%S", &ltm);
    file << reportDate << "\n";
    file << "================================================================\n\n";

    // 1. Revenue Overview
    file << "1. REVENUE OVERVIEW\n";
    file << "----------------------------------------------------------------\n";

    std::string totalQuery = "SELECT SUM(amount) as total FROM payments";
    sql::ResultSet* totalRes = db->executeQuery(totalQuery);
    if (totalRes && totalRes->next()) {
        double total = totalRes->isNull("total") ? 0.0 : totalRes->getDouble("total");
        file << "Total Revenue:        RM" << std::fixed << std::setprecision(2) << total << "\n";
    }
    if (totalRes) delete totalRes;

    std::string monthQuery = "SELECT SUM(amount) as monthly FROM payments "
        "WHERE MONTH(payment_date) = MONTH(CURDATE()) AND YEAR(payment_date) = YEAR(CURDATE())";
    sql::ResultSet* monthRes = db->executeQuery(monthQuery);
    if (monthRes && monthRes->next()) {
        double monthly = monthRes->isNull("monthly") ? 0.0 : monthRes->getDouble("monthly");
        file << "This Month's Revenue: RM" << std::fixed << std::setprecision(2) << monthly << "\n";
    }
    if (monthRes) delete monthRes;

    std::string todayQuery = "SELECT SUM(amount) as today FROM payments WHERE payment_date = CURDATE()";
    sql::ResultSet* todayRes = db->executeQuery(todayQuery);
    if (todayRes && todayRes->next()) {
        double today = todayRes->isNull("today") ? 0.0 : todayRes->getDouble("today");
        file << "Today's Revenue:      RM" << std::fixed << std::setprecision(2) << today << "\n";
    }
    if (todayRes) delete todayRes;

    std::string countQuery = "SELECT COUNT(*) as count FROM payments";
    sql::ResultSet* countRes = db->executeQuery(countQuery);
    if (countRes && countRes->next()) {
        file << "Total Transactions:   " << countRes->getInt("count") << "\n";
    }
    if (countRes) delete countRes;

    std::string activeQuery = "SELECT COUNT(*) as active FROM members WHERE payment_status = 'paid'";
    sql::ResultSet* activeRes = db->executeQuery(activeQuery);
    if (activeRes && activeRes->next()) {
        file << "Active Members:       " << activeRes->getInt("active") << "\n";
    }
    if (activeRes) delete activeRes;

    // 2. Monthly Revenue Report
    file << "\n2. MONTHLY REVENUE REPORT (Last 12 Months)\n";
    file << "----------------------------------------------------------------\n";
    file << std::left << std::setw(15) << "MONTH" << std::setw(15) << "REVENUE" << std::setw(15) << "TRANSACTIONS" << "\n";
    file << "----------------------------------------------------------------\n";

    std::string monthlyQuery = "SELECT DATE_FORMAT(payment_date, '%Y-%m') as month, "
        "SUM(amount) as total, COUNT(*) as transactions "
        "FROM payments "
        "GROUP BY DATE_FORMAT(payment_date, '%Y-%m') "
        "ORDER BY month DESC "
        "LIMIT 12";

    sql::ResultSet* monthlyRes = db->executeQuery(monthlyQuery);
    if (monthlyRes) {
        while (monthlyRes->next()) {
            file << std::left << std::setw(15) << monthlyRes->getString("month")
                << "RM" << std::setw(14) << std::fixed << std::setprecision(2) << monthlyRes->getDouble("total")
                << std::setw(15) << monthlyRes->getInt("transactions") << "\n";
        }
        delete monthlyRes;
    }

    // 3. Payment Method Analysis
    file << "\n3. PAYMENT METHOD ANALYSIS\n";
    file << "----------------------------------------------------------------\n";
    file << std::left << std::setw(20) << "METHOD" << std::setw(15) << "TRANSACTIONS" << std::setw(15) << "TOTAL" << "\n";
    file << "----------------------------------------------------------------\n";

    std::string methodQuery = "SELECT payment_method, COUNT(*) as count, SUM(amount) as total "
        "FROM payments GROUP BY payment_method ORDER BY total DESC";

    sql::ResultSet* methodRes = db->executeQuery(methodQuery);
    if (methodRes) {
        while (methodRes->next()) {
            file << std::left << std::setw(20) << methodRes->getString("payment_method")
                << std::setw(15) << methodRes->getInt("count")
                << "RM" << std::fixed << std::setprecision(2) << methodRes->getDouble("total") << "\n";
        }
        delete methodRes;
    }

    // 4. Top Members by Spending
    file << "\n4. TOP 10 MEMBERS BY SPENDING\n";
    file << "----------------------------------------------------------------\n";
    file << std::left << std::setw(5) << "RANK" << std::setw(25) << "NAME" << std::setw(15) << "TOTAL SPENT" << std::setw(12) << "PAYMENTS" << "\n";
    file << "----------------------------------------------------------------\n";

    std::string topQuery = "SELECT m.member_id, m.first_name, m.last_name, "
        "SUM(p.amount) as total_spent, COUNT(p.payment_id) as payments "
        "FROM members m JOIN payments p ON m.member_id = p.member_id "
        "GROUP BY m.member_id, m.first_name, m.last_name "
        "ORDER BY total_spent DESC LIMIT 10";

    sql::ResultSet* topRes = db->executeQuery(topQuery);
    if (topRes) {
        int rank = 1;
        while (topRes->next()) {
            std::string fullName = topRes->getString("first_name") + " " + topRes->getString("last_name");
            file << std::left << std::setw(5) << rank++
                << std::setw(25) << fullName.substr(0, 23)
                << "RM" << std::setw(14) << std::fixed << std::setprecision(2) << topRes->getDouble("total_spent")
                << std::setw(12) << topRes->getInt("payments") << "\n";
        }
        delete topRes;
    }

    // 5. Membership Type Distribution
    file << "\n5. MEMBERSHIP TYPE DISTRIBUTION\n";
    file << "----------------------------------------------------------------\n";
    file << std::left << std::setw(20) << "PLAN" << std::setw(15) << "MEMBERS" << std::setw(15) << "REVENUE" << "\n";
    file << "----------------------------------------------------------------\n";


    // Count members per plan and sum revenue
    std::string typeQuery = "SELECT t.type_name, COUNT(m.member_id) as member_count, "
        "COALESCE(SUM(p.amount), 0) as revenue "
        "FROM membership_types t "
        "LEFT JOIN members m ON t.type_id = m.membership_type_id "
        "LEFT JOIN payments p ON m.member_id = p.member_id "
        "GROUP BY t.type_id, t.type_name "
        "ORDER BY revenue DESC";

    sql::ResultSet* typeRes = db->executeQuery(typeQuery);
    if (typeRes) {
        while (typeRes->next()) {
            file << std::left << std::setw(20) << typeRes->getString("type_name")
                << std::setw(15) << typeRes->getInt("member_count")
                << "RM" << std::fixed << std::setprecision(2) << typeRes->getDouble("revenue") << "\n";
        }
        delete typeRes;
    }

    // Footer
    file << "\n================================================================\n";
    file << "                    END OF REPORT\n";
    file << "================================================================\n";

    file.close();
    std::cout << "\nReport exported successfully to: " << filename << "\n";
}

// ============================================
// EXPIRING MEMBERSHIPS
// ============================================
void Admin::checkExpiringMemberships() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "      EXPIRING MEMBERSHIPS\n";
        std::cout << "========================================\n";
        std::cout << "1. Expiring in 7 Days\n";
        std::cout << "2. Expiring in 30 Days\n";
        std::cout << "3. Already Expired\n";
        std::cout << "4. Export to CSV\n";
        std::cout << "5. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        std::string query;
        std::string title;

        switch (choice) {
        case 1:
            query = "SELECT m.member_id, m.first_name, m.last_name, m.email, m.phone, "
                "m.expiry_date, t.type_name, DATEDIFF(m.expiry_date, CURDATE()) as days_left "
                "FROM members m "
                "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
                "WHERE m.expiry_date IS NOT NULL "
                "AND DATEDIFF(m.expiry_date, CURDATE()) BETWEEN 0 AND 7 "
                "ORDER BY m.expiry_date";
            title = "EXPIRING WITHIN 7 DAYS";
            break;
        case 2:
            query = "SELECT m.member_id, m.first_name, m.last_name, m.email, m.phone, "
                "m.expiry_date, t.type_name, DATEDIFF(m.expiry_date, CURDATE()) as days_left " // Calculate days until expiry
                "FROM members m "
                "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
                "WHERE m.expiry_date IS NOT NULL "
                "AND DATEDIFF(m.expiry_date, CURDATE()) BETWEEN 0 AND 30 "
                "ORDER BY m.expiry_date";
            title = "EXPIRING WITHIN 30 DAYS";
            break;
        case 3:
            query = "SELECT m.member_id, m.first_name, m.last_name, m.email, m.phone, "
                "m.expiry_date, t.type_name, DATEDIFF(CURDATE(), m.expiry_date) as days_expired " // Calculate days since expiry
                "FROM members m "
                "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
                "WHERE m.expiry_date IS NOT NULL "
                "AND m.expiry_date < CURDATE() "
                "ORDER BY m.expiry_date DESC";
            title = "ALREADY EXPIRED";
            break;
        case 4:
            std::cout << "Export members expiring within how many days? ";
            int days;
            std::cin >> days;
            exportExpiringToCSV(days);
            continue;
        case 5:
            return;
        default:
            std::cout << "Invalid choice!\n";
            continue;
        }

        sql::ResultSet* res = db->executeQuery(query);

        if (res) {
            std::cout << "\n=== " << title << " ===\n";
            std::cout << std::left
                << std::setw(5) << "ID"
                << std::setw(20) << "NAME"
                << std::setw(25) << "EMAIL"
                << std::setw(12) << "EXPIRY"
                << std::setw(10) << "DAYS" << "\n";
            std::cout << std::string(72, '-') << "\n";

            int count = 0;
            while (res->next()) {
                count++;
                std::string fullName = res->getString("first_name") + " " + res->getString("last_name");
                std::string expiry = res->isNull("expiry_date") ? "N/A" : res->getString("expiry_date");

                std::cout << std::left
                    << std::setw(5) << res->getInt("member_id")
                    << std::setw(20) << fullName.substr(0, 18)
                    << std::setw(25) << res->getString("email").substr(0, 23)
                    << std::setw(12) << expiry;

                if (choice == 3) {
                    std::cout << res->getInt("days_expired") << " ago";
                }
                else {
                    std::cout << res->getInt("days_left") << " left";
                }
                std::cout << "\n";
            }

            std::cout << std::string(72, '-') << "\n";
            std::cout << "Total: " << count << " member(s)\n";
            delete res;
        }
    } while (choice != 5);
}

void Admin::exportExpiringToCSV(int daysThreshold) {
    std::string query = "SELECT m.member_id, m.first_name, m.last_name, m.email, m.phone, "
        "m.expiry_date, t.type_name "
        "FROM members m "
        "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
        "WHERE m.expiry_date IS NOT NULL "
        "AND DATEDIFF(m.expiry_date, CURDATE()) BETWEEN 0 AND " + std::to_string(daysThreshold) +
        " ORDER BY m.expiry_date";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::string filename = "expiring_members_" + std::to_string(daysThreshold) + "days.csv";
        std::ofstream file(filename);

        if (file.is_open()) {
            file << "Member ID,First Name,Last Name,Email,Phone,Expiry Date,Plan\n";

            int count = 0;
            while (res->next()) {
                count++;
                file << res->getInt("member_id") << ","
                    << escapeCSV(res->getString("first_name")) << ","
                    << escapeCSV(res->getString("last_name")) << ","
                    << escapeCSV(res->getString("email")) << ","
                    << escapeCSV(res->getString("phone")) << ","
                    << res->getString("expiry_date") << ","
                    << (res->isNull("type_name") ? "None" : escapeCSV(res->getString("type_name"))) << "\n";
            }

            file.close();
            std::cout << "Exported " << count << " members to " << filename << "\n";
        }
        else {
            std::cout << "Failed to create CSV file!\n";
        }
        delete res;
    }
}

// ============================================
// PAYMENT MANAGEMENT
// ============================================
void Admin::managePayments() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "         MANAGE PAYMENTS\n";
        std::cout << "========================================\n";
        std::cout << "1. View All Payments\n";
        std::cout << "2. Add Payment\n";
        std::cout << "3. Delete Payment\n";
        std::cout << "4. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: viewPayments(); break;
        case 2: addPayment(); break;
        case 3: deletePayment(); break;
        case 4: break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 4);
}

void Admin::viewPayments() {
    std::cout << "\n========================================\n";
    std::cout << "         ALL PAYMENTS\n";
    std::cout << "========================================\n";

    std::string query = "SELECT p.payment_id, p.member_id, m.first_name, m.last_name, "
        "p.amount, p.payment_method, p.payment_date "
        "FROM payments p "
        "JOIN members m ON p.member_id = m.member_id "
        "ORDER BY p.payment_date DESC "
        "LIMIT 50";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(8) << "PAY ID"
            << std::setw(20) << "MEMBER"
            << std::setw(12) << "AMOUNT"
            << std::setw(15) << "METHOD"
            << std::setw(12) << "DATE" << "\n";
        std::cout << std::string(67, '-') << "\n";

        double total = 0.0;
        while (res->next()) {
            std::string fullName = res->getString("first_name") + " " + res->getString("last_name");
            double amount = res->getDouble("amount");
            total += amount;

            std::cout << std::left
                << std::setw(8) << res->getInt("payment_id")
                << std::setw(20) << fullName.substr(0, 18)
                << "RM" << std::setw(11) << std::fixed << std::setprecision(2) << amount
                << std::setw(15) << res->getString("payment_method")
                << std::setw(12) << res->getString("payment_date") << "\n";
        }

        std::cout << std::string(67, '-') << "\n";
        std::cout << "Total Displayed: RM" << std::fixed << std::setprecision(2) << total << "\n";
        delete res;
    }
}

void Admin::addPayment() {
    std::cout << "\n========================================\n";
    std::cout << "         ADD PAYMENT\n";
    std::cout << "========================================\n";

    viewAllMembers();

    std::cout << "\nEnter Member ID: ";
    int memberId;
    std::cin >> memberId;

    // Verify member exists
    std::string checkQuery = "SELECT first_name, last_name FROM members WHERE member_id = " + std::to_string(memberId);
    sql::ResultSet* checkRes = db->executeQuery(checkQuery);

    if (!checkRes || !checkRes->next()) {
        std::cout << "Member not found!\n";
        if (checkRes) delete checkRes;
        return;
    }

    std::cout << "Member: " << checkRes->getString("first_name") << " " << checkRes->getString("last_name") << "\n";
    delete checkRes;

    std::cout << "Amount: RM";
    double amount;
    std::cin >> amount;

    std::cout << "Payment Method:\n";
    std::cout << "1. Cash\n";
    std::cout << "2. Credit Card\n";
    std::cout << "3. Debit Card\n";
    std::cout << "4. Bank Transfer\n";
    std::cout << "Enter choice: ";

    int methodChoice;
    std::cin >> methodChoice;

    std::string paymentMethod;
    switch (methodChoice) {
    case 1: paymentMethod = "Cash"; break;
    case 2: paymentMethod = "Credit Card"; break;
    case 3: paymentMethod = "Debit Card"; break;
    case 4: paymentMethod = "Bank Transfer"; break;
    default: paymentMethod = "Cash";
    }

    std::string insertQuery = "INSERT INTO payments (member_id, amount, payment_method, payment_date) VALUES (" +
        std::to_string(memberId) + ", " + std::to_string(amount) + ", '" + paymentMethod + "', CURDATE())";

    if (db->executeUpdate(insertQuery)) {
        // Update member payment status
        std::string updateQuery = "UPDATE members SET payment_status = 'paid' WHERE member_id = " + std::to_string(memberId);
        db->executeUpdate(updateQuery);

        std::cout << "Payment added successfully!\n";
    }
    else {
        std::cout << "Failed to add payment!\n";
    }
}

void Admin::deletePayment() {
    std::cout << "\n========================================\n";
    std::cout << "         DELETE PAYMENT\n";
    std::cout << "========================================\n";

    viewPayments();

    std::cout << "\nEnter Payment ID to delete (0 to cancel): ";
    int paymentId;
    std::cin >> paymentId;

    if (paymentId == 0) return;

    std::cout << "Are you sure you want to delete this payment? (1=Yes, 0=No): ";
    int confirm;
    std::cin >> confirm;

    if (confirm != 1) {
        std::cout << "Deletion cancelled.\n";
        return;
    }

    std::string deleteQuery = "DELETE FROM payments WHERE payment_id = " + std::to_string(paymentId);

    if (db->executeUpdate(deleteQuery)) {
        std::cout << "Payment deleted successfully!\n";
    }
    else {
        std::cout << "Failed to delete payment!\n";
    }
}

// ============================================
// ATTENDANCE
// ============================================
void Admin::viewAttendance() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "         VIEW ATTENDANCE\n";
        std::cout << "========================================\n";
        std::cout << "1. Today's Attendance\n";
        std::cout << "2. All Attendance Records\n";
        std::cout << "3. Search by Member\n";
        std::cout << "4. Attendance Statistics\n";
        std::cout << "5. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        std::string query;

        switch (choice) {
        case 1:
            query = "SELECT a.*, m.first_name, m.last_name "
                "FROM attendance a "
                "JOIN members m ON a.member_id = m.member_id "
                "WHERE DATE(a.check_in_time) = CURDATE() "
                "ORDER BY a.check_in_time DESC";
            break;
        case 2:
            query = "SELECT a.*, m.first_name, m.last_name "
                "FROM attendance a "
                "JOIN members m ON a.member_id = m.member_id "
                "ORDER BY a.check_in_time DESC "
                "LIMIT 50";
            break;
        case 3: {
            std::cout << "Enter Member ID: ";
            int searchMemberId;
            std::cin >> searchMemberId;
            query = "SELECT a.*, m.first_name, m.last_name "
                "FROM attendance a "
                "JOIN members m ON a.member_id = m.member_id "
                "WHERE a.member_id = " + std::to_string(searchMemberId) +
                " ORDER BY a.check_in_time DESC LIMIT 20";
            break;
        }
        case 4: {
            // Attendance statistics
            std::cout << "\n=== ATTENDANCE STATISTICS ===\n";

            std::string todayQuery = "SELECT COUNT(*) as count FROM attendance WHERE DATE(check_in_time) = CURDATE()";
            sql::ResultSet* todayRes = db->executeQuery(todayQuery);
            if (todayRes && todayRes->next()) {
                std::cout << "Today's Check-ins:    " << todayRes->getInt("count") << "\n";
            }
            if (todayRes) delete todayRes;

            std::string weekQuery = "SELECT COUNT(*) as count FROM attendance WHERE check_in_time >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)";
            sql::ResultSet* weekRes = db->executeQuery(weekQuery);
            if (weekRes && weekRes->next()) {
                std::cout << "This Week's Visits:   " << weekRes->getInt("count") << "\n";
            }
            if (weekRes) delete weekRes;

            std::string monthQuery = "SELECT COUNT(*) as count FROM attendance WHERE MONTH(check_in_time) = MONTH(CURDATE())";
            sql::ResultSet* monthRes = db->executeQuery(monthQuery);
            if (monthRes && monthRes->next()) {
                std::cout << "This Month's Visits:  " << monthRes->getInt("count") << "\n";
            }
            if (monthRes) delete monthRes;

            continue;
        }
        case 5:
            return;
        default:
            std::cout << "Invalid choice!\n";
            continue;
        }

        sql::ResultSet* res = db->executeQuery(query);

        if (res) {
            std::cout << std::left
                << std::setw(10) << "MEMBER"
                << std::setw(20) << "NAME"
                << std::setw(20) << "CHECK-IN"
                << std::setw(20) << "CHECK-OUT" << "\n";
            std::cout << std::string(70, '-') << "\n";

            int count = 0;
            while (res->next()) {
                count++;
                std::string fullName = res->getString("first_name") + " " + res->getString("last_name");
                std::string checkOut = res->isNull("check_out_time") ? "Still in" : res->getString("check_out_time");

                std::cout << std::left
                    << std::setw(10) << res->getInt("member_id")
                    << std::setw(20) << fullName.substr(0, 18)
                    << std::setw(20) << res->getString("check_in_time")
                    << std::setw(20) << checkOut << "\n";
            }

            std::cout << std::string(70, '-') << "\n";
            std::cout << "Total Records: " << count << "\n";
            delete res;
        }
    } while (choice != 5);
}

// ============================================
// MEMBERSHIP TYPES MANAGEMENT
// ============================================
void Admin::manageMembershipTypes() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "      MANAGE MEMBERSHIP TYPES\n";
        std::cout << "========================================\n";
        std::cout << "1. View All Types\n";
        std::cout << "2. Add New Type\n";
        std::cout << "3. Update Type\n";
        std::cout << "4. Delete Type\n";
        std::cout << "5. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: viewMembershipTypes(); break;
        case 2: addMembershipType(); break;
        case 3: updateMembershipType(); break;
        case 4: deleteMembershipType(); break;
        case 5: break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 5);
}

void Admin::viewMembershipTypes() {
    std::cout << "\n========================================\n";
    std::cout << "      ALL MEMBERSHIP TYPES\n";
    std::cout << "========================================\n";

    std::string query = "SELECT t.*, "
        "(SELECT COUNT(*) FROM members WHERE membership_type_id = t.type_id) as member_count "
        "FROM membership_types t ORDER BY t.type_id";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(5) << "ID"
            << std::setw(15) << "NAME"
            << std::setw(12) << "PRICE"
            << std::setw(12) << "DURATION"
            << std::setw(10) << "MEMBERS" << "\n";
        std::cout << std::string(54, '-') << "\n";

        while (res->next()) {
            int months = res->getInt("duration_months");
            std::string duration = std::to_string(months) + " month" + (months > 1 ? "s" : "");

            std::cout << std::left
                << std::setw(5) << res->getInt("type_id")
                << std::setw(15) << res->getString("type_name")
                << "RM" << std::setw(11) << std::fixed << std::setprecision(2) << res->getDouble("price")
                << std::setw(12) << duration
                << std::setw(10) << res->getInt("member_count") << "\n";
        }
        std::cout << std::string(54, '-') << "\n";
        delete res;
    }
}

void Admin::addMembershipType() {
    std::cout << "\n========================================\n";
    std::cout << "      ADD NEW MEMBERSHIP TYPE\n";
    std::cout << "========================================\n";

    std::cin.ignore();
    std::cout << "Type Name: ";
    std::string typeName;
    std::getline(std::cin, typeName);

    std::cout << "Price: RM";
    double price;
    std::cin >> price;

    std::cout << "Duration (months): ";
    int duration;
    std::cin >> duration;

    std::string insertQuery = "INSERT INTO membership_types (type_name, price, duration_months) VALUES ('" +
        typeName + "', " + std::to_string(price) + ", " + std::to_string(duration) + ")";

    if (db->executeUpdate(insertQuery)) {
        std::cout << "Membership type added successfully!\n";
    }
    else {
        std::cout << "Failed to add membership type!\n";
    }
}

void Admin::updateMembershipType() {
    std::cout << "\n========================================\n";
    std::cout << "      UPDATE MEMBERSHIP TYPE\n";
    std::cout << "========================================\n";

    viewMembershipTypes();

    std::cout << "\nEnter Type ID to update (0 to cancel): ";
    int typeId;
    std::cin >> typeId;

    if (typeId == 0) return;

    std::cout << "What to update?\n";
    std::cout << "1. Name\n";
    std::cout << "2. Price\n";
    std::cout << "3. Duration\n";
    std::cout << "Enter choice: ";

    int updateChoice;
    std::cin >> updateChoice;

    std::string updateQuery;

    switch (updateChoice) {
    case 1: {
        std::cin.ignore();
        std::cout << "New Name: ";
        std::string newName;
        std::getline(std::cin, newName);
        updateQuery = "UPDATE membership_types SET type_name = '" + newName + "' WHERE type_id = " + std::to_string(typeId);
        break;
    }
    case 2: {
        std::cout << "New Price: RM";
        double newPrice;
        std::cin >> newPrice;
        updateQuery = "UPDATE membership_types SET price = " + std::to_string(newPrice) + " WHERE type_id = " + std::to_string(typeId);
        break;
    }
    case 3: {
        std::cout << "New Duration (months): ";
        int newDuration;
        std::cin >> newDuration;
        updateQuery = "UPDATE membership_types SET duration_months = " + std::to_string(newDuration) + " WHERE type_id = " + std::to_string(typeId);
        break;
    }
    default:
        std::cout << "Invalid choice!\n";
        return;
    }

    if (db->executeUpdate(updateQuery)) {
        std::cout << "Membership type updated successfully!\n";
    }
    else {
        std::cout << "Failed to update membership type!\n";
    }
}

void Admin::deleteMembershipType() {
    std::cout << "\n========================================\n";
    std::cout << "      DELETE MEMBERSHIP TYPE\n";
    std::cout << "========================================\n";

    viewMembershipTypes();

    std::cout << "\nEnter Type ID to delete (0 to cancel): ";
    int typeId;
    std::cin >> typeId;

    if (typeId == 0) return;

    // Check if any members are using this type
    std::string checkQuery = "SELECT COUNT(*) as count FROM members WHERE membership_type_id = " + std::to_string(typeId);
    sql::ResultSet* checkRes = db->executeQuery(checkQuery);

    if (checkRes && checkRes->next()) {
        int memberCount = checkRes->getInt("count");
        if (memberCount > 0) {
            std::cout << "Warning: " << memberCount << " member(s) are using this membership type.\n";
            std::cout << "Deleting will set their membership type to NULL.\n";
        }
    }
    if (checkRes) delete checkRes;

    std::cout << "Are you sure you want to delete this type? (1=Yes, 0=No): ";
    int confirm;
    std::cin >> confirm;

    if (confirm != 1) {
        std::cout << "Deletion cancelled.\n";
        return;
    }

    // Set members with this type to NULL first
    std::string updateMembers = "UPDATE members SET membership_type_id = NULL WHERE membership_type_id = " + std::to_string(typeId);
    db->executeUpdate(updateMembers);

    std::string deleteQuery = "DELETE FROM membership_types WHERE type_id = " + std::to_string(typeId);

    if (db->executeUpdate(deleteQuery)) {
        std::cout << "Membership type deleted successfully!\n";
    }
    else {
        std::cout << "Failed to delete membership type!\n";
    }
}

// ============================================
// ADMIN APPROVALS
// ============================================
void Admin::manageAdminApprovals() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "       MANAGE ADMIN APPROVALS\n";
        std::cout << "========================================\n";
        std::cout << "1. View Pending Admins\n";
        std::cout << "2. Approve Admin\n";
        std::cout << "3. Reject Admin\n";
        std::cout << "4. View All Admins\n";
        std::cout << "5. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: viewPendingAdmins(); break;
        case 2: {
            viewPendingAdmins();
            std::cout << "\nEnter User ID to approve (0 to cancel): ";
            int approveId;
            std::cin >> approveId;
            if (approveId > 0) approveAdmin(approveId);
            break;
        }
        case 3: {
            viewPendingAdmins();
            std::cout << "\nEnter User ID to reject (0 to cancel): ";
            int rejectId;
            std::cin >> rejectId;
            if (rejectId > 0) rejectAdmin(rejectId);
            break;
        }
        case 4: viewApprovedAdmins(); break;
        case 5: break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 5);
}

void Admin::viewPendingAdmins() {
    std::cout << "\n========================================\n";
    std::cout << "        PENDING ADMIN REQUESTS\n";
    std::cout << "========================================\n";

    std::string query = "SELECT u.user_id, u.name, u.email, u.phone "
        "FROM users u "
        "JOIN admin a ON u.email = a.username "
        "WHERE a.is_approved = 0";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(8) << "USER ID"
            << std::setw(25) << "NAME"
            << std::setw(30) << "EMAIL"
            << std::setw(15) << "PHONE" << "\n";
        std::cout << std::string(78, '-') << "\n";

        int count = 0;
        while (res->next()) {
            count++;
            std::cout << std::left
                << std::setw(8) << res->getInt("user_id")
                << std::setw(25) << res->getString("name").substr(0, 23)
                << std::setw(30) << res->getString("email").substr(0, 28)
                << std::setw(15) << res->getString("phone") << "\n";
        }

        if (count == 0) {
            std::cout << "No pending admin requests.\n";
        }
        else {
            std::cout << std::string(78, '-') << "\n";
            std::cout << "Total Pending: " << count << "\n";
        }
        delete res;
    }
}

void Admin::approveAdmin(int userId) {
    // Get admin email from user_id
    std::string emailQuery = "SELECT email FROM users WHERE user_id = " + std::to_string(userId);
    sql::ResultSet* emailRes = db->executeQuery(emailQuery);

    if (!emailRes || !emailRes->next()) {
        std::cout << "User not found!\n";
        if (emailRes) delete emailRes;
        return;
    }

    std::string email = emailRes->getString("email");
    delete emailRes;

    // Update admin table
    std::string updateAdmin = "UPDATE admin SET is_approved = 1 WHERE username = '" + email + "'";

    if (db->executeUpdate(updateAdmin)) {
        std::cout << "Admin approved successfully!\n";
        std::cout << "User can now login with admin privileges.\n";
    }
    else {
        std::cout << "Failed to approve admin!\n";
    }
}

void Admin::rejectAdmin(int userId) {
    // Get admin email from user_id
    std::string emailQuery = "SELECT email FROM users WHERE user_id = " + std::to_string(userId);
    sql::ResultSet* emailRes = db->executeQuery(emailQuery);

    if (!emailRes || !emailRes->next()) {
        std::cout << "User not found!\n";
        if (emailRes) delete emailRes;
        return;
    }

    std::string email = emailRes->getString("email");
    delete emailRes;

    std::cout << "Are you sure you want to reject and delete this admin request? (1=Yes, 0=No): ";
    int confirm;
    std::cin >> confirm;

    if (confirm != 1) {
        std::cout << "Rejection cancelled.\n";
        return;
    }

    // Delete from admin table
    std::string deleteAdmin = "DELETE FROM admin WHERE username = '" + email + "'";
    db->executeUpdate(deleteAdmin);

    // Delete from users table
    std::string deleteUser = "DELETE FROM users WHERE user_id = " + std::to_string(userId);

    if (db->executeUpdate(deleteUser)) {
        std::cout << "Admin request rejected and deleted.\n";
    }
    else {
        std::cout << "Failed to reject admin!\n";
    }
}

void Admin::viewApprovedAdmins() {
    std::cout << "\n========================================\n";
    std::cout << "         ALL APPROVED ADMINS\n";
    std::cout << "========================================\n";

    std::string query = "SELECT u.user_id, u.name, u.email, u.phone "
        "FROM users u "
        "JOIN admin a ON u.email = a.username "
        "WHERE a.is_approved = 1";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left
            << std::setw(8) << "USER ID"
            << std::setw(25) << "NAME"
            << std::setw(30) << "EMAIL"
            << std::setw(15) << "PHONE" << "\n";
        std::cout << std::string(78, '-') << "\n";

        int count = 0;
        while (res->next()) {
            count++;
            std::cout << std::left
                << std::setw(8) << res->getInt("user_id")
                << std::setw(25) << res->getString("name").substr(0, 23)
                << std::setw(30) << res->getString("email").substr(0, 28)
                << std::setw(15) << res->getString("phone") << "\n";
        }

        std::cout << std::string(78, '-') << "\n";
        std::cout << "Total Admins: " << count << "\n";
        delete res;
    }
}

// ============================================
// HELPER METHODS
// ============================================
std::string Admin::escapeCSV(const std::string& str) {
    std::string result = str;
    // If string contains comma, quote, or newline, wrap in quotes
    if (result.find(',') != std::string::npos ||
        result.find('"') != std::string::npos ||
        result.find('\n') != std::string::npos) {
        // Escape existing quotes by doubling them
        size_t pos = 0;
        while ((pos = result.find('"', pos)) != std::string::npos) {
            result.replace(pos, 1, "\"\"");
            pos += 2;
        }
        result = "\"" + result + "\"";
    }
    return result;
}
