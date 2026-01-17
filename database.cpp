#include "Database.h"
#include <iostream>

// Static instance
Database* Database::instance = nullptr;

Database::Database() {
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(
            "tcp://127.0.0.1:3306",
            "root",
            ""   // change if needed
        );
        con->setSchema("gym_management");

        std::cout << "[OK] MySQL Connected Successfully!\n";
    }
    catch (sql::SQLException& e) {
        std::cout << "[ERROR] MySQL Connection Failed\n";
        std::cout << e.what() << std::endl;
        con = nullptr;
    }
}

Database* Database::getInstance() {
    if (!instance)
        instance = new Database();
    return instance;
}

sql::Connection* Database::getConnection() {
    return con;
}

bool Database::isConnected() {
    return con != nullptr && con->isValid();
}

sql::ResultSet* Database::executeQuery(const std::string& query) {
    try {
        if (!con || !con->isValid()) {
            std::cout << "[ERROR] Database connection is not valid!\n";
            return nullptr;
        }
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery(query);
        delete stmt;
        return res;
    }
    catch (sql::SQLException& e) {
        std::cout << "[ERROR] Query failed: " << e.what() << "\n";
        return nullptr;
    }
}

bool Database::executeUpdate(const std::string& query) {
    try {
        if (!con || !con->isValid()) {
            std::cout << "[ERROR] Database connection is not valid!\n";
            return false;
        }
        sql::Statement* stmt = con->createStatement();
        stmt->execute(query);
        delete stmt;
        return true;
    }
    catch (sql::SQLException& e) {
        std::cout << "[ERROR] Update failed: " << e.what() << "\n";
        return false;
    }
}

void Database::closeConnection() {
    if (con) {
        con->close();
        delete con;
        con = nullptr;
        std::cout << "[INFO] Database connection closed.\n";
    }
}
