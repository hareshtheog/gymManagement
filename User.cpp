#include "User.h"

// Default constructor
User::User() : userId(0), name(""), email(""), password("") {}

// Parameterized constructor
User::User(int id, const std::string& name, const std::string& email, const std::string& password)
    : userId(id), name(name), email(email), password(password) {}
