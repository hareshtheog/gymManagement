#include "Member.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <conio.h>

Member::Member() : User(), memberId(0), db(nullptr) {
    db = Database::getInstance();
}

Member::Member(int id, const std::string& name, const std::string& email,
    const std::string& password, int memberId)
    : User(id, name, email, password), memberId(memberId), db(nullptr) {
    db = Database::getInstance();
}

void Member::displayMenu() {
    int choice;
    do {
        std::cout << "\n========================================\n";
        std::cout << "            MEMBER MENU\n";
        std::cout << "========================================\n";
        std::cout << "1. View Profile\n";
        std::cout << "2. Update Profile\n";
        std::cout << "3. Make Payment\n";
        std::cout << "4. View Payment History\n";
        std::cout << "5. Gym Attendance\n";
        std::cout << "6. Check Membership Status\n";
        std::cout << "7. Renew Membership\n";
        std::cout << "8. Logout\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: viewProfile(); break;
        case 2: updateProfile(); break;
        case 3: makePayment(); break;
        case 4: viewPaymentHistory(); break;
        case 5: manageAttendance(); break;
        case 6: checkMembershipStatus(); break;
        case 7: renewMembership(); break;
        case 8: std::cout << "Logging out...\n"; break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 8);
}

void Member::viewProfile() {
    std::string query = "SELECT * FROM members WHERE member_id = " + std::to_string(memberId);
    sql::ResultSet* res = db->executeQuery(query);

    if (res && res->next()) {
        std::cout << "\n=== YOUR PROFILE ===\n";
        std::cout << "Member ID: " << res->getInt("member_id") << "\n";
        std::cout << "Name: " << res->getString("first_name") << " " << res->getString("last_name") << "\n";
        std::cout << "Email: " << res->getString("email") << "\n";
        std::cout << "Phone: " << res->getString("phone") << "\n";
        std::cout << "Join Date: " << res->getString("join_date") << "\n";
        std::cout << "Payment Status: " << res->getString("payment_status") << "\n";
    }
    else {
        std::cout << "Profile not found!\n";
    }

    if (res) delete res;
}

void Member::updateProfile() {
    std::string phone;
    std::cout << "Enter new phone number: ";
    std::cin >> phone;

    std::string query = "UPDATE members SET phone = '" + phone + "' WHERE member_id = " + std::to_string(memberId);
    if (db->executeUpdate(query)) {
        std::cout << "Profile updated successfully!\n";
    }
    else {
        std::cout << "Failed to update profile!\n";
    }
}

void Member::makePayment() {
    std::cout << "\n========================================\n";
    std::cout << "           MAKE PAYMENT\n";
    std::cout << "========================================\n";

    // Check current membership
    std::string query = "SELECT m.membership_type_id, t.type_name, t.price, t.duration_months FROM members m "
        "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
        "WHERE m.member_id = " + std::to_string(memberId);

    sql::ResultSet* res = db->executeQuery(query);

    if (res && res->next()) {
        if (res->isNull("membership_type_id")) {
            std::cout << "You don't have an active membership plan.\n";
            std::cout << "Please select a membership plan first (Option 7: Renew Membership).\n";
            if (res) delete res;
            return;
        }

        std::string typeName = res->getString("type_name");
        double price = res->getDouble("price");
        int duration = res->getInt("duration_months");
        std::string typeId = res->getString("membership_type_id");

        std::cout << "\n--- Payment Details ---\n";
        std::cout << "Membership Plan: " << typeName << "\n";
        std::cout << "Duration:        " << duration << " month(s)\n";
        std::cout << "Amount Due:      RM" << std::fixed << std::setprecision(2) << price << "\n";
        std::cout << "------------------------\n";

        std::cout << "\nSelect Payment Method:\n";
        std::cout << "1. Cash\n";
        std::cout << "2. Credit Card\n";
        std::cout << "3. Debit Card\n";
        std::cout << "4. Cancel\n";
        std::cout << "Enter choice: ";

        int methodChoice;
        std::cin >> methodChoice;
        std::cin.ignore();

        std::string paymentMethod;
        bool paymentConfirmed = false;

        switch (methodChoice) {
        case 1: {
            // CASH PAYMENT
            std::cout << "\n--- CASH PAYMENT ---\n";
            std::cout << "Amount to pay: RM" << std::fixed << std::setprecision(2) << price << "\n";
            std::cout << "\nPlease hand over the cash to the receptionist.\n";

            std::cout << "Enter amount received: RM";
            double amountReceived;
            std::cin >> amountReceived;


            // Validate amount received
            if (amountReceived < price) {
                std::cout << "Insufficient amount! Payment cancelled.\n";
                if (res) delete res;
                return;
            }

            // Cash payment change calculation
            double change = amountReceived - price;
            if (change > 0) {
                std::cout << "Change to return: RM" << std::fixed << std::setprecision(2) << change << "\n";
            }

            std::cout << "\nConfirm cash payment? (1=Yes, 0=No): ";
            int confirm;
            std::cin >> confirm;

            if (confirm == 1) {
                paymentMethod = "Cash";
                paymentConfirmed = true;
            }
            break;
        }
        case 2: {
            // CREDIT CARD PAYMENT
            std::cout << "\n--- CREDIT CARD PAYMENT ---\n";
            std::cout << "Amount: RM" << std::fixed << std::setprecision(2) << price << "\n\n";

            std::string cardNumber, expiryDate, cvv, cardHolder;

            std::cout << "Cardholder Name: ";
            std::getline(std::cin, cardHolder);

            std::cout << "Card Number (16 digits): ";
            std::getline(std::cin, cardNumber);

            // Validate card number length
            if (cardNumber.length() != 16) {
                std::cout << "Invalid card number! Must be 16 digits.\n";
                if (res) delete res;
                return;
            }

            std::cout << "Expiry Date (MM/YY): ";
            std::getline(std::cin, expiryDate);

            // Validate expiry format
            if (expiryDate.length() != 5 || expiryDate[2] != '/') {
                std::cout << "Invalid expiry date format!\n";
                if (res) delete res;
                return;
            }

            std::cout << "CVV (3 digits): ";
            std::getline(std::cin, cvv);

            if (cvv.length() != 3) {
                std::cout << "Invalid CVV! Must be 3 digits.\n";
                if (res) delete res;
                return;
            }

            // Display masked card info
            std::string maskedCard = "**** **** **** " + cardNumber.substr(12, 4);
            std::cout << "\n--- Confirm Payment ---\n";
            std::cout << "Card: " << maskedCard << "\n";
            std::cout << "Name: " << cardHolder << "\n";
            std::cout << "Amount: RM" << std::fixed << std::setprecision(2) << price << "\n";

            std::cout << "\nProcess payment? (1=Yes, 0=No): ";
            int confirm;
            std::cin >> confirm;

            if (confirm == 1) {
                std::cout << "\nProcessing credit card payment";
                for (int i = 0; i < 3; i++) {
                    std::cout << ".";
                    std::cout.flush();
                }
                std::cout << "\n";

                // Simulate processing
                paymentMethod = "Credit Card";
                paymentConfirmed = true;
            }
            break;
        }
        case 3: {
            // DEBIT CARD PAYMENT
            std::cout << "\n--- DEBIT CARD PAYMENT ---\n";
            std::cout << "Amount: RM" << std::fixed << std::setprecision(2) << price << "\n\n";

            std::string cardNumber, pin;

            std::cout << "Debit Card Number (16 digits): ";
            std::getline(std::cin, cardNumber);

            if (cardNumber.length() != 16) {
                std::cout << "Invalid card number! Must be 16 digits.\n";
                if (res) delete res;
                return;
            }

            std::cout << "Enter PIN: ";
            // Read PIN with masking
            pin = "";
            char ch;
            while ((ch = _getch()) != 13) { // Enter key
                if (ch == 8) { // Backspace
                    if (!pin.empty()) {
                        pin.pop_back();
                        std::cout << "\b \b";
                    }
                }
                else {
                    pin += ch;
                    std::cout << "*";
                }
            }
            std::cout << "\n";

            if (pin.length() < 4 || pin.length() > 6) {
                std::cout << "Invalid PIN! Must be 4-6 digits.\n";
                if (res) delete res;
                return;
            }

            // Display masked card info
            std::string maskedCard = "**** **** **** " + cardNumber.substr(12, 4);
            std::cout << "\n--- Confirm Payment ---\n";
            std::cout << "Card: " << maskedCard << "\n";
            std::cout << "Amount: RM" << std::fixed << std::setprecision(2) << price << "\n";

            std::cout << "\nProcess payment? (1=Yes, 0=No): ";
            int confirm;
            std::cin >> confirm;

            if (confirm == 1) {
                std::cout << "\nVerifying PIN and processing";
                for (int i = 0; i < 3; i++) {
                    std::cout << ".";
                    std::cout.flush();
                }
                std::cout << "\n";

                paymentMethod = "Debit Card";
                paymentConfirmed = true;
            }
            break;
        }
        case 4:
            std::cout << "Payment cancelled.\n";
            if (res) delete res;
            return;
        default:
            std::cout << "Invalid choice! Payment cancelled.\n";
            if (res) delete res;
            return;
        }

        if (paymentConfirmed) {
            // Insert payment record
            std::string paymentQuery = "INSERT INTO payments (member_id, amount, payment_method, payment_date) VALUES (" +
                std::to_string(memberId) + ", " + std::to_string(price) + ", '" +
                paymentMethod + "', CURDATE())";

            if (db->executeUpdate(paymentQuery)) {
                // Update member's payment status
                std::string updateQuery = "UPDATE members SET payment_status = 'paid', start_date = CURDATE(), "
                    "expiry_date = DATE_ADD(CURDATE(), INTERVAL " + std::to_string(duration) + " MONTH) "
                    "WHERE member_id = " + std::to_string(memberId);

                if (db->executeUpdate(updateQuery)) {
                    std::cout << "\n========================================\n";
                    std::cout << "       PAYMENT SUCCESSFUL!\n";
                    std::cout << "========================================\n";
                    std::cout << "Payment Method: " << paymentMethod << "\n";
                    std::cout << "Amount Paid:    RM" << std::fixed << std::setprecision(2) << price << "\n";
                    std::cout << "Plan:           " << typeName << "\n";
                    std::cout << "Duration:       " << duration << " month(s)\n";
                    std::cout << "Status:         ACTIVE\n";
                    std::cout << "========================================\n";
                    std::cout << "Thank you for your payment!\n";
                }
            }
            else {
                std::cout << "Payment processing failed! Please try again.\n";
            }
        }
        else {
            std::cout << "Payment cancelled.\n";
        }
    }
    else {
        std::cout << "Error retrieving membership information.\n";
    }

    if (res) delete res;
}

void Member::viewPaymentHistory() {
    std::cout << "\n=== PAYMENT HISTORY ===\n";

    std::string query = "SELECT * FROM payments WHERE member_id = " + std::to_string(memberId) +
        " ORDER BY payment_date DESC";

    sql::ResultSet* res = db->executeQuery(query);

    if (res) {
        std::cout << std::left << std::setw(12) << "Date"
            << std::setw(10) << "Amount"
            << std::setw(15) << "Method" << "\n";
        std::cout << std::string(40, '-') << "\n";

        bool hasPayments = false;
        double total = 0.0;

        while (res->next()) {
            hasPayments = true;
            std::string date = res->getString("payment_date");
            double amount = res->getDouble("amount");
            std::string method = res->getString("payment_method");

            total += amount;

            std::cout << std::left << std::setw(12) << date
                << "RM" << std::setw(9) << std::fixed << std::setprecision(2) << amount
                << std::setw(15) << method << "\n";
        }

        if (hasPayments) {
            std::cout << std::string(40, '-') << "\n";
            std::cout << "Total Paid: RM" << std::fixed << std::setprecision(2) << total << "\n";
        }
        else {
            std::cout << "No payment history found.\n";
        }

        delete res;
    }
}

void Member::manageAttendance() {
    int choice;
    do {
        // Check current status (if already checked in)
        bool isCheckedIn = false;
        std::string checkInTime = "";

        try {
            std::string statusQuery = "SELECT check_in_time FROM attendance WHERE member_id = " + std::to_string(memberId) +
                " AND check_out_time IS NULL ORDER BY check_in_time DESC LIMIT 1";
            sql::ResultSet* statusRes = db->executeQuery(statusQuery);

            if (statusRes && statusRes->next()) {
                isCheckedIn = true;
                checkInTime = statusRes->getString("check_in_time");
            }
            if (statusRes) delete statusRes;
        }
        catch (...) {
            // Ignore errors checking status
        }

        std::cout << "\n========================================\n";
        std::cout << "          GYM ATTENDANCE\n";
        std::cout << "========================================\n";

        if (isCheckedIn) {
            std::cout << "Status: [CHECKED IN]\n";
            std::cout << "Since:  " << checkInTime << "\n";
        }
        else {
            std::cout << "Status: [NOT CHECKED IN]\n";
        }

        std::cout << "----------------------------------------\n";
        std::cout << "1. Check In\n";
        std::cout << "2. Check Out\n";
        std::cout << "3. View Attendance History\n";
        std::cout << "4. View Statistics\n";
        std::cout << "5. Back to Main Menu\n";
        std::cout << "========================================\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: checkIn(); break;
        case 2: checkOut(); break;
        case 3: viewAttendance(); break;
        case 4: {
            // View statistics
            std::cout << "\n=== ATTENDANCE STATISTICS ===\n";

            try {
                std::string totalQuery = "SELECT COUNT(*) as total FROM attendance WHERE member_id = " + std::to_string(memberId);
                sql::ResultSet* totalRes = db->executeQuery(totalQuery);
                if (totalRes && totalRes->next()) {
                    std::cout << "Total Visits:      " << totalRes->getInt("total") << "\n";
                }
                if (totalRes) delete totalRes;

                std::string monthQuery = "SELECT COUNT(*) as monthly FROM attendance WHERE member_id = " + std::to_string(memberId) +
                    " AND MONTH(check_in_time) = MONTH(CURDATE()) AND YEAR(check_in_time) = YEAR(CURDATE())";
                sql::ResultSet* monthRes = db->executeQuery(monthQuery);
                if (monthRes && monthRes->next()) {
                    std::cout << "This Month:        " << monthRes->getInt("monthly") << " visits\n";
                }
                if (monthRes) delete monthRes;

                std::string weekQuery = "SELECT COUNT(*) as weekly FROM attendance WHERE member_id = " + std::to_string(memberId) +
                    " AND check_in_time >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)";
                sql::ResultSet* weekRes = db->executeQuery(weekQuery);
                if (weekRes && weekRes->next()) {
                    std::cout << "This Week:         " << weekRes->getInt("weekly") << " visits\n";
                }
                if (weekRes) delete weekRes;



                // Average session duration calculation
                std::string avgQuery = "SELECT AVG(TIMESTAMPDIFF(MINUTE, check_in_time, check_out_time)) as avg_duration "
                    "FROM attendance WHERE member_id = " + std::to_string(memberId) + " AND check_out_time IS NOT NULL";
                sql::ResultSet* avgRes = db->executeQuery(avgQuery);
                if (avgRes && avgRes->next() && !avgRes->isNull("avg_duration")) {
                    int avgMinutes = avgRes->getInt("avg_duration");
                    int hours = avgMinutes / 60;
                    int mins = avgMinutes % 60;
                    std::cout << "Avg Session Time:  " << hours << "h " << mins << "m\n";
                }
                if (avgRes) delete avgRes;
            }
            catch (...) {
                std::cout << "Unable to load statistics.\n";
            }

            break;
        }
        case 5: break;
        default: std::cout << "Invalid choice!\n";
        }
    } while (choice != 5);
}

void Member::checkIn() {
    try {
        // Check if membership is active
        std::string membershipQuery = "SELECT payment_status, expiry_date FROM members WHERE member_id = " + std::to_string(memberId);
        sql::ResultSet* membershipRes = db->executeQuery(membershipQuery);

        if (membershipRes && membershipRes->next()) {
            std::string status = membershipRes->getString("payment_status");

            if (status != "paid") {
                std::cout << "\n[ERROR] Your membership is not active!\n";
                std::cout << "Please make a payment to activate your membership.\n";
                delete membershipRes;
                return;
            }

            // Check if membership is expired
            if (!membershipRes->isNull("expiry_date")) {
                std::string expiryCheckQuery = "SELECT DATEDIFF('" + membershipRes->getString("expiry_date") + "', CURDATE()) as days_left";
                sql::ResultSet* expiryRes = db->executeQuery(expiryCheckQuery);
                if (expiryRes && expiryRes->next()) {
                    int daysLeft = expiryRes->getInt("days_left");
                    if (daysLeft < 0) {
                        std::cout << "\n[ERROR] Your membership has expired!\n";
                        std::cout << "Please renew your membership to check in.\n";
                        delete expiryRes;
                        delete membershipRes;
                        return;
                    }
                }
                if (expiryRes) delete expiryRes;
            }
        }
        if (membershipRes) delete membershipRes;

        // Check if already checked in
        std::string checkQuery = "SELECT check_in_time FROM attendance WHERE member_id = " + std::to_string(memberId) +
            " AND check_out_time IS NULL LIMIT 1";
        sql::ResultSet* checkRes = db->executeQuery(checkQuery);

        if (checkRes && checkRes->next()) {
            std::cout << "\n[WARNING] You are already checked in!\n";
            std::cout << "Check-in time: " << checkRes->getString("check_in_time") << "\n";
            std::cout << "Please check out first before checking in again.\n";
            delete checkRes;
            return;
        }
        if (checkRes) delete checkRes;

        // Perform check-in
        std::string insertQuery = "INSERT INTO attendance (member_id, check_in_time) VALUES (" +
            std::to_string(memberId) + ", NOW())";

        if (db->executeUpdate(insertQuery)) {
            // Get the check-in time
            std::string timeQuery = "SELECT NOW() as current_time";
            sql::ResultSet* timeRes = db->executeQuery(timeQuery);
            std::string currentTime = "";
            if (timeRes && timeRes->next()) {
                currentTime = timeRes->getString("current_time");
            }
            if (timeRes) delete timeRes;

            std::cout << "\n========================================\n";
            std::cout << "       CHECK-IN SUCCESSFUL!\n";
            std::cout << "========================================\n";
            std::cout << "Time: " << currentTime << "\n";
            std::cout << "----------------------------------------\n";
            std::cout << "Welcome to the gym! Have a great workout!\n";
            std::cout << "========================================\n";
        }
        else {
            std::cout << "\n[ERROR] Check-in failed! Please try again.\n";
        }
    }
    catch (...) {
        std::cout << "\n[ERROR] An error occurred during check-in.\n";
    }
}

void Member::checkOut() {
    try {
        // Check if currently checked in
        std::string checkQuery = "SELECT check_in_time FROM attendance WHERE member_id = " + std::to_string(memberId) +
            " AND check_out_time IS NULL ORDER BY check_in_time DESC LIMIT 1";
        sql::ResultSet* checkRes = db->executeQuery(checkQuery);

        if (!checkRes || !checkRes->next()) {
            std::cout << "\n[WARNING] You are not currently checked in!\n";
            std::cout << "Please check in first.\n";
            if (checkRes) delete checkRes;
            return;
        }

        std::string checkInTime = checkRes->getString("check_in_time");
        delete checkRes;

        // Perform check-out - update by member_id and check_in_time time
        std::string updateQuery = "UPDATE attendance SET check_out_time = NOW() WHERE member_id = " + std::to_string(memberId) +
            " AND check_in_time = '" + checkInTime + "' AND check_out_time IS NULL";

        if (db->executeUpdate(updateQuery)) {
            // Calculate duration
            std::string durationQuery = "SELECT check_in_time, check_out_time, TIMESTAMPDIFF(MINUTE, check_in_time, check_out_time) as duration "
                "FROM attendance WHERE member_id = " + std::to_string(memberId) +
                " AND check_in_time = '" + checkInTime + "'";
            sql::ResultSet* durationRes = db->executeQuery(durationQuery);

            std::cout << "\n========================================\n";
            std::cout << "       CHECK-OUT SUCCESSFUL!\n";
            std::cout << "========================================\n";

            if (durationRes && durationRes->next()) {
                std::string checkOutTime = durationRes->getString("check_out_time");
                int durationMins = durationRes->getInt("duration");
                int hours = durationMins / 60;
                int mins = durationMins % 60;

                std::cout << "Check-in:  " << checkInTime << "\n";
                std::cout << "Check-out: " << checkOutTime << "\n";
                std::cout << "Duration:  " << hours << " hour(s) " << mins << " minute(s)\n";
            }
            if (durationRes) delete durationRes;

            std::cout << "----------------------------------------\n";
            std::cout << "Thank you for visiting! See you soon!\n";
            std::cout << "========================================\n";
        }
        else {
            std::cout << "\n[ERROR] Check-out failed! Please try again.\n";
        }
    }
    catch (...) {
        std::cout << "\n[ERROR] An error occurred during check-out.\n";
    }
}

void Member::viewAttendance() {
    std::cout << "\n========================================\n";
    std::cout << "       ATTENDANCE HISTORY\n";
    std::cout << "========================================\n";

    try {
        std::string query = "SELECT check_in_time, check_out_time, "
            "TIMESTAMPDIFF(MINUTE, check_in_time, check_out_time) as duration_mins " // Calculate session duration in minutes
            "FROM attendance WHERE member_id = " + std::to_string(memberId) +
            " ORDER BY check_in_time DESC LIMIT 20";

        sql::ResultSet* res = db->executeQuery(query);

        if (res) {
            std::cout << std::left << std::setw(12) << "DATE"
                << std::setw(10) << "CHECK-IN"
                << std::setw(10) << "CHECK-OUT"
                << std::setw(12) << "DURATION" << "\n";
            std::cout << std::string(44, '-') << "\n";

            bool hasAttendance = false;
            int totalVisits = 0;
            int totalMinutes = 0;
            int completedVisits = 0;

            while (res->next()) {
                hasAttendance = true;
                totalVisits++;

                std::string checkInStr = res->getString("check_in_time");
                std::string date = checkInStr.substr(0, 10);
                std::string checkIn = checkInStr.substr(11, 5);

                std::string checkOut = "Still in";
                std::string duration = "-";

                if (!res->isNull("check_out_time")) {
                    std::string checkOutStr = res->getString("check_out_time");
                    checkOut = checkOutStr.substr(11, 5);

                    if (!res->isNull("duration_mins")) {
                        int mins = res->getInt("duration_mins");
                        totalMinutes += mins;
                        completedVisits++;
                        int h = mins / 60;
                        int m = mins % 60;
                        duration = std::to_string(h) + "h " + std::to_string(m) + "m";
                    }
                }

                std::cout << std::left << std::setw(12) << date
                    << std::setw(10) << checkIn
                    << std::setw(10) << checkOut
                    << std::setw(12) << duration << "\n";
            }

            if (hasAttendance) {
                std::cout << std::string(44, '-') << "\n";
                std::cout << "Total Visits: " << totalVisits << "\n";
                if (completedVisits > 0 && totalMinutes > 0) {
                    int avgMins = totalMinutes / completedVisits; // Calculate average from total
                    std::cout << "Avg Duration: " << (avgMins / 60) << "h " << (avgMins % 60) << "m\n";
                }
            }
            else {
                std::cout << "No attendance records found.\n";
            }

            delete res;
        }
        else {
            std::cout << "No attendance records found.\n";
        }
    }
    catch (...) {
        std::cout << "Unable to load attendance records.\n";
    }
    std::cout << "========================================\n";
}

void Member::checkMembershipStatus() {
    std::string query = "SELECT m.payment_status, t.type_name, m.start_date, m.expiry_date FROM members m "
        "LEFT JOIN membership_types t ON m.membership_type_id = t.type_id "
        "WHERE m.member_id = " + std::to_string(memberId);

    sql::ResultSet* res = db->executeQuery(query);

    if (res && res->next()) {
        std::string status = res->getString("payment_status");
        std::string typeName = res->isNull("type_name") ? "No Membership" : res->getString("type_name");
        std::string startDate = res->isNull("start_date") ? "N/A" : res->getString("start_date");
        std::string expiry = res->isNull("expiry_date") ? "N/A" : res->getString("expiry_date");

        std::cout << "\n=== MEMBERSHIP STATUS ===\n";
        std::cout << "Status: " << status << "\n";
        std::cout << "Plan: " << typeName << "\n";
        std::cout << "Start Date: " << startDate << "\n";
        std::cout << "Expiry Date: " << expiry << "\n";

        // Check if membership is expired
        if (expiry != "N/A") {
            std::string checkQuery = "SELECT DATEDIFF('" + expiry + "', CURDATE()) as days_left";
            sql::ResultSet* checkRes = db->executeQuery(checkQuery);

            if (checkRes && checkRes->next()) {
                int daysLeft = checkRes->getInt("days_left");
                if (daysLeft < 0) {
                    std::cout << "[WARNING] Your membership has EXPIRED!\n";
                }
                else if (daysLeft <= 7) {
                    std::cout << "[WARNING] Your membership expires in " << daysLeft << " days!\n";
                }
                else {
                    std::cout << "[OK] " << daysLeft << " days remaining\n";
                }
            }

            if (checkRes) delete checkRes;
        }
    }
    else {
        std::cout << "Could not retrieve membership status.\n";
    }

    if (res) delete res;
}

void Member::renewMembership() {
    std::cout << "\n=== RENEW MEMBERSHIP ===\n";

    // Show available membership types
    std::string typeQuery = "SELECT type_id, type_name, price, duration_months FROM membership_types ORDER BY type_id";
    sql::ResultSet* typeRes = db->executeQuery(typeQuery);

    if (typeRes) {
        std::cout << "\n=== AVAILABLE PLANS ===\n";
        std::cout << std::left << std::setw(5) << "ID"
            << std::setw(15) << "PLAN"
            << std::setw(15) << "PRICE"
            << std::setw(15) << "DURATION" << "\n";
        std::cout << std::string(50, '-') << "\n";

        while (typeRes->next()) {
            int typeId = typeRes->getInt("type_id");
            std::string planName = typeRes->getString("type_name");
            double price = typeRes->getDouble("price");
            int durationMonths = typeRes->getInt("duration_months");

            std::string durationStr;
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
                << "RM" << std::setw(14) << std::fixed << std::setprecision(2) << price
                << std::setw(15) << durationStr << "\n";
        }
        delete typeRes;

        std::cout << std::string(50, '-') << "\n";
        std::cout << "Select Plan ID to renew (0 to cancel): ";

        int planId;
        std::cin >> planId;

        if (planId > 0) {
            // Update membership type
            std::string updateQuery = "UPDATE members SET membership_type_id = " + std::to_string(planId) +
                ", payment_status = 'pending' WHERE member_id = " + std::to_string(memberId);

            if (db->executeUpdate(updateQuery)) {
                std::cout << "\n[SUCCESS] Membership plan updated!\n";
                std::cout << "Please make payment to activate your new membership.\n";

                // Ask if user wants to make payment now
                std::cout << "\nMake payment now? (1=Yes, 0=No): ";
                int payNow;
                std::cin >> payNow;

                if (payNow == 1) {
                    makePayment();
                }
            }
            else {
                std::cout << "Failed to update membership plan.\n";
            }
        }
    }
    else {
        std::cout << "Unable to load membership plans.\n";
    }
}