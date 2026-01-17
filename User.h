#ifndef USER_H
#define USER_H

#include <string>

class User {
protected:
    int userId;
    std::string name;
    std::string email;
    std::string password;

public:
    User();  // Default constructor
    User(int id, const std::string& name, const std::string& email, const std::string& password);
    virtual ~User() {}

    virtual void displayMenu() = 0;

    // Getters
    int getUserId() const { return userId; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    std::string getPassword() const { return password; }

    // Setters
    void setUserId(int id) { userId = id; }
    void setName(const std::string& name) { this->name = name; }
    void setEmail(const std::string& email) { this->email = email; }
    void setPassword(const std::string& password) { this->password = password; }
};

#endif