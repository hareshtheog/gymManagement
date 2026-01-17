#ifndef MEMBER_H
#define MEMBER_H

#include "User.h"
#include "Database.h"
#include <string>

class Member : public User {
private:
    int memberId;
    Database* db;

    // Member-specific methods
    void viewProfile();
    void updateProfile();
    void makePayment();
    void viewPaymentHistory();
    void manageAttendance();
    void viewAttendance();
    void checkIn();
    void checkOut();
    void checkMembershipStatus();
    void renewMembership();

public:
    Member();
    Member(int id, const std::string& name, const std::string& email,
        const std::string& password, int memberId);
    void displayMenu() override;

    int getMemberId() const { return memberId; }
};

#endif