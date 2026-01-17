#include "AuthSystem.h"
#include "Database.h"
#include "User.h"
#include "Admin.h"
#include "Member.h"
#include <iostream>
#include <conio.h>
#include <iomanip>
#include <limits>
#include <string>

AuthSystem::AuthSystem() {
    db = Database::getInstance();
}

std::string AuthSystem::getHiddenPassword() {
    std::string password = "";
    char ch;

    while (true) {
        ch = _getch();
        if (ch == 13) { // Enter key
            std::cout << std::endl;
            break;
        }
        else if (ch == 8) { // Backspace
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        }
        else {
            password += ch;
            std::cout << "*";
        }
    }
    return password;
}

bool AuthSystem::emailExists(const std::string& email) {
    std::string query = "SELECT COUNT(*) as count FROM users WHERE email = '" + email + "'";
    sql::ResultSet* res = db->executeQuery(query);

    if (!res) return false;

    bool exists = false;
    if (res->next()) {
        exists = res->getInt("count") > 0;
    }

    delete res;
    return exists;
}

int AuthSystem::getMemberIdByEmail(const std::string& email) {
    std::string query = "SELECT m.member_id FROM members m " +
        std::string("JOIN users u ON m.user_id = u.user_id ") +
        "WHERE u.email = '" + email + "'";

    sql::ResultSet* res = db->executeQuery(query);

    int memberId = 0;
    if (res && res->next()) {
        memberId = res->getInt("member_id");
    }

    delete res;
    return memberId;
}

bool AuthSystem::registerUser() {
    std::string email, password, confirmPassword, role;
    std::string firstName, lastName, phone, fullName;
    int membershipTypeId = 0;

    std::cout << "\n=== REGISTRATION ===\n";

    std::cout << "Choose User Type:\n";
    std::cout << "1. Member\n";
    std::cout << "2. Admin (requires admin approval)\n";
    std::cout << "Enter choice (1-2): ";

    int typeChoice;
    std::cin >> typeChoice;

    if (typeChoice == 1) {
        role = "member";
    }
    else if (typeChoice == 2) {
        role = "admin";
    }
    else {
        std::cout << "Invalid choice!\n";
        return false;
    }

    std::cin.ignore();

    // For members, collect additional info
    if (role == "member") {
        std::cout << "\n=== MEMBER INFORMATION ===\n";
        std::cout << "First Name: ";
        std::getline(std::cin, firstName);

        std::cout << "Last Name: ";
        std::getline(std::cin, lastName);

        std::cout << "Phone: ";
        std::getline(std::cin, phone);

        std::cout << "Email: ";
        std::getline(std::cin, email);

        fullName = firstName + " " + lastName;

        // Show membership types
        std::string typeQuery = "SELECT type_id, type_name, price, duration_months FROM membership_types ORDER BY type_id";
        sql::ResultSet* typeRes = db->executeQuery(typeQuery);

        if (typeRes) {
            std::cout << "\n=== MEMBERSHIP PLANS ===\n";
            std::cout << std::left << std::setw(5) << "ID"
                << std::setw(15) << "PLAN"
                << std::setw(15) << "PRICE"
                << std::setw(15) << "DURATION"
                << std::setw(20) << "MONTHLY COST" << "\n";
            std::cout << std::string(70, '-') << "\n";

            bool hasData = false;
            while (typeRes->next()) {
                hasData = true;
                int typeId = typeRes->getInt("type_id");
                std::string planName = typeRes->getString("type_name");
                double price = typeRes->getDouble("price");
                int durationMonths = typeRes->getInt("duration_months");

                std::string durationStr;
                // Calculates monthly cost from total price and duration
                double monthlyCost = price / durationMonths;

                if (durationMonths == 1) {
                    durationStr = "1 Month";
                }
                else if (durationMonths == 12) {
                    durationStr = "1 Year";
                }
                else {
                    durationStr = std::to_string(durationMonths) + " Months";
                }

                std::cout << std::left << std::setw(5) << typeId
                    << std::setw(15) << planName
                    << "$" << std::setw(14) << std::fixed << std::setprecision(2) << price
                    << std::setw(15) << durationStr
                    << "$" << std::setw(19) << std::fixed << std::setprecision(2) << monthlyCost << "\n";
            }
            delete typeRes;

            if (!hasData) {
                std::cout << "No membership plans available.\n";
                std::cout << "Please select 0 to continue without a membership.\n";
            }

            std::cout << std::string(70, '-') << "\n";
            std::cout << "0 - No membership (register as guest member)\n";
            std::cout << std::string(70, '-') << "\n";
        }
        else {
            std::cout << "\nUnable to load membership plans.\n";
        }

        std::cout << "\nMembership Plan ID (0 for none): ";
        while (!(std::cin >> membershipTypeId)) {
            std::cout << "Invalid input! Please enter a number (0, 1, or 2): ";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
        std::cin.ignore();

        // Add validation for valid membership IDs
        if (membershipTypeId != 0) {
            std::string checkQuery = "SELECT COUNT(*) as exists_count FROM membership_types WHERE type_id = " + std::to_string(membershipTypeId);
            sql::ResultSet* checkRes = db->executeQuery(checkQuery);

            if (checkRes && checkRes->next()) {
                int exists = checkRes->getInt("exists_count");
                if (exists == 0) {
                    std::cout << "Invalid membership ID! Setting to 'none'.\n";
                    membershipTypeId = 0;
                }
            }
            if (checkRes) delete checkRes;
        }
    }
    else {
        // For admin registration
        std::cout << "\n=== ADMIN REGISTRATION ===\n";
        std::cout << "Full Name: ";
        std::getline(std::cin, fullName);

        std::cout << "Email: ";
        std::getline(std::cin, email);

        std::cout << "Phone: ";
        std::getline(std::cin, phone);
    }

    // ========== PASSWORD COLLECTION ==========
    std::cout << "Password: ";
    password = getHiddenPassword();

    std::cout << "Confirm Password: ";
    confirmPassword = getHiddenPassword();

    if (password != confirmPassword) {
        std::cout << "Passwords do not match!\n";
        return false;
    }

    if (password.length() < 6) {
        std::cout << "Password must be at least 6 characters long!\n";
        return false;
    }

    // Check if email already exists
    if (emailExists(email)) {
        std::cout << "Email already registered!\n";
        return false;
    }

    // Insert into users table
    std::string userQuery = "INSERT INTO users (name, email, password, phone, role) VALUES ('" +
        fullName + "', '" + email + "', '" + password + "', '" + phone + "', '" + role + "')";

    if (!db->executeUpdate(userQuery)) {
        std::cout << "Registration failed!\n";
        return false;
    }
    std::cout << "Registration successful!\n";

    // GET THE NEW USER ID
    std::string getUserIdQuery = "SELECT LAST_INSERT_ID() as new_user_id";
    sql::ResultSet* userIdRes = db->executeQuery(getUserIdQuery);
    int newUserId = 0;

    if (userIdRes && userIdRes->next()) {
        newUserId = userIdRes->getInt("new_user_id");
    }

    if (userIdRes) delete userIdRes;

    if (role == "admin") {
        // Insert into admin table with PENDING approval
        std::string adminQuery = "INSERT INTO admin (username, password, role, is_approved) VALUES ('" +
            email + "', '" + password + "', 'admin', 0)";

        if (db->executeUpdate(adminQuery)) {
            // Also update users table with pending status
            std::string updateUserQuery = "UPDATE users SET is_approved = 0 WHERE email = '" + email + "'";
            db->executeUpdate(updateUserQuery);

            std::cout << "\nAdmin registration submitted successfully!\n";
            std::cout << "Your account requires approval from an existing admin.\n";
            std::cout << "Please wait for approval before logging in.\n";
        }
        else {
            std::cout << "Warning: Could not save admin details to admin table.\n";
        }
        return true;
    }

    // Handle member creation
    if (role == "member") {
        if (newUserId == 0) {
            std::cout << "CRITICAL ERROR: newUserId is 0! Cannot create member record.\n";
            return true;
        }

        // GET NEXT MEMBER ID
        std::string maxIdQuery = "SELECT IFNULL(MAX(member_id), 0) + 1 as next_id FROM members";
        sql::ResultSet* maxIdRes = db->executeQuery(maxIdQuery);
        int nextMemberId = 1;

        if (maxIdRes && maxIdRes->next()) {
            nextMemberId = maxIdRes->getInt("next_id");
        }
        if (maxIdRes) delete maxIdRes;

        // MODIFIED INSERT QUERY - MySQL version
        std::string memberQuery = "INSERT INTO members (member_id, user_id, first_name, last_name, phone, email, membership_type_id, join_date, start_date, expiry_date, payment_status) " +
            std::string("VALUES (") +
            std::to_string(nextMemberId) + ", " +      // member_id
            std::to_string(newUserId) + ", '" +       // user_id
            firstName + "', '" +                      // first_name
            lastName + "', '" +                       // last_name
            phone + "', '" +                          // phone
            email + "', " +                           // email
            (membershipTypeId > 0 ? std::to_string(membershipTypeId) : "NULL") + ", " + // membership_type_id
            "CURDATE(), " +                          // join_date (CHANGED: GETDATE() -> CURDATE())
            "NULL, " +                               // start_date
            "NULL, " +                               // expiry_date
            "'pending')";

        if (db->executeUpdate(memberQuery)) {
            std::cout << "\nSUCCESS: Member registration complete!\n";
            if (membershipTypeId > 0) {
                std::cout << "IMPORTANT: Your membership payment is PENDING.\n";
                std::cout << "Please login and make payment to activate your membership.\n";
            }
        }
        else {
            std::cout << "\nERROR: Failed to create member record!\n";
            std::cout << "User account was created but member record failed.\n";
        }

        return true;
    }

    return false;
}

User* AuthSystem::login() {
    std::string email, password;

    std::cout << "\n=== LOGIN ===\n";
    std::cout << "Email: ";
    std::cin >> email;

    std::cout << "Password: ";
    password = getHiddenPassword();

    std::string query = "SELECT user_id, name, email, password, role FROM users WHERE email = '" +
        email + "' AND password = '" + password + "'";

    sql::ResultSet* res = db->executeQuery(query);

    if (!res) {
        std::cout << "\nLogin failed! (Query execution error)\n";
        return nullptr;
    }

    if (res->next()) {
        int userId = res->getInt("user_id");
        std::string dbName = res->getString("name");
        std::string dbEmail = res->getString("email");
        std::string dbPassword = res->getString("password");
        std::string userRole = res->getString("role");

        std::cout << "\nLogin successful! Welcome " << dbName << "!\n";

        User* user = nullptr;

        if (userRole == "admin") {
            // Check if admin is approved
            std::string approvalQuery = "SELECT is_approved FROM admin WHERE username = '" + dbEmail + "'";
            sql::ResultSet* approvalRes = db->executeQuery(approvalQuery);

            if (approvalRes && approvalRes->next()) {
                bool isApproved = approvalRes->getBoolean("is_approved");
                if (isApproved) {
                    user = new Admin(userId, dbName, dbEmail, dbPassword);
                    std::cout << "Admin privileges verified.\n";
                }
                else {
                    std::cout << "\nYour admin account is PENDING APPROVAL.\n";
                    std::cout << "Please wait for an existing admin to approve your account.\n";
                    delete res;
                    delete approvalRes;
                    return nullptr;
                }
            }
            else {
                std::cout << "Error: Could not verify admin status.\n";
                delete res;
                return nullptr;
            }
            delete approvalRes;
        }
        else if (userRole == "member") {
            int memberId = getMemberIdByEmail(dbEmail);
            if (memberId > 0) {
                user = new Member(userId, dbName, dbEmail, dbPassword, memberId);
            }
            else {
                std::cout << "Error: Member record not found for this user.\n";
            }
        }
        else {
            std::cout << "Error: Unknown user role '" << userRole << "'\n";
        }

        delete res;
        return user;
    }

    delete res;
    std::cout << "\nInvalid email or password!\n";
    return nullptr;
}

void AuthSystem::resetPassword() {
    std::string email, newPassword, confirmPassword;

    std::cout << "\n=== RESET PASSWORD ===\n";
    std::cout << "Email: ";
    std::cin >> email;

    if (!emailExists(email)) {
        std::cout << "Email not found!\n";
        return;
    }

    std::cout << "New Password: ";
    newPassword = getHiddenPassword();

    std::cout << "Confirm New Password: ";
    confirmPassword = getHiddenPassword();

    if (newPassword != confirmPassword) {
        std::cout << "Passwords do not match!\n";
        return;
    }

    if (newPassword.length() < 6) {
        std::cout << "Password must be at least 6 characters long!\n";
        return;
    }

    // Update password in users table
    std::string query = "UPDATE users SET password = '" + newPassword + "' WHERE email = '" + email + "'";
    std::string adminQuery = "UPDATE admin SET password = '" + newPassword + "' WHERE username = '" + email + "'";

    bool userUpdated = db->executeUpdate(query);
    bool adminUpdated = db->executeUpdate(adminQuery); // This will fail if not an admin, but that's OK

    if (userUpdated) {
        std::cout << "Password reset successfully!\n";
    }
    else {
        std::cout << "Failed to reset password in users table!\n";
    }
}