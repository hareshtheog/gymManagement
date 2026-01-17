// AuthSystem.h - FIXED VERSION
#ifndef AUTHSYSTEM_H
#define AUTHSYSTEM_H

#include <string>

// Forward declarations
class Database;
class User;

class AuthSystem {
private:
    Database* db;

    std::string getHiddenPassword();
    bool emailExists(const std::string& email);
    int getMemberIdByEmail(const std::string& email);

public:
    AuthSystem();
    bool registerUser();
    User* login();
    void resetPassword();
};

#endif