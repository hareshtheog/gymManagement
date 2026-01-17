#ifndef DATABASE_H
#define DATABASE_H

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>

class Database {
private:
    static Database* instance;
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;

    Database();

public:
    static Database* getInstance();
    sql::Connection* getConnection();
    bool isConnected();

    sql::ResultSet* executeQuery(const std::string& query);
    bool executeUpdate(const std::string& query);

    void closeConnection();
};

#endif
