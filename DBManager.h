#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <iostream>
#include <string>
#include <vector>
#include "sqlite3.h"

struct User {
    int id;
    std::string email;
    bool isAdmin;
};

class DBManager {
private:
    sqlite3* db;
    char* zErrMsg;

public:
    DBManager(const std::string& dbName) : db(nullptr), zErrMsg(nullptr) {
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            db = nullptr;
        }
    }

    ~DBManager() {
        if (db) {
            sqlite3_close(db);
        }
    }

    sqlite3* getDB() const {
        return db;
    }

    bool executeQuery(const std::string& sql, int (*callback)(void*, int, char**, char**) = nullptr, void* data = nullptr) {
        if (!db) return false;
        int rc = sqlite3_exec(db, sql.c_str(), callback, data, &zErrMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
            return false;
        }
        return true;
    }

    void initializeDatabase() {
        std::string sql = "CREATE TABLE IF NOT EXISTS properties ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT, location TEXT, price REAL, type TEXT, "
                          "isAvailable INTEGER, InfoNumber TEXT);"
                          
                          "CREATE TABLE IF NOT EXISTS users ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "email TEXT UNIQUE, password TEXT, isAdmin INTEGER);"
                          
                          "CREATE TABLE IF NOT EXISTS appointments ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "user_id INTEGER NOT NULL, "
                          "property_id INTEGER NOT NULL, "
                          "appointment_date TEXT NOT NULL, "
                          "status TEXT DEFAULT 'pending', "
                          "notes TEXT, "
                          "FOREIGN KEY (user_id) REFERENCES users(id), "
                          "FOREIGN KEY (property_id) REFERENCES properties(id));"

                          "INSERT OR IGNORE INTO properties (name, location, price, type, isAvailable, InfoNumber) VALUES "
                          "('Ocean View Villa', 'Malibu', 1250000.0, 'Buy', 1, '555-0101'),"
                          "('Downtown Apt', 'New York', 3500.0, 'Rent', 1, '555-0202'),"
                          "('Mountain Cabin', 'Aspen', 450000.0, 'Buy', 0, '555-0303');"
                          
                          
                          "INSERT OR IGNORE INTO users (email, password, isAdmin) VALUES "
                          "('admin@system.com', 'admin123', 1),"
                          "('user@system.com', 'user123', 0);";

        executeQuery(sql);
    }

    bool validateUser(const std::string& email, const std::string& password, int& userId, bool& isAdmin) {
        std::string sql = "SELECT id, isAdmin FROM users WHERE email = ? AND password = ?;";
        sqlite3_stmt* stmt;
        bool success = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                userId = sqlite3_column_int(stmt, 0);
                isAdmin = sqlite3_column_int(stmt, 1) != 0;
                success = true;
            }
        }
        sqlite3_finalize(stmt);
        return success;
    }

    // Authentication function
    bool authenticate(const std::string& email, const std::string& password, User& user) {
        if (!db) return false;
        
        std::string sql = "SELECT id, email, isAdmin FROM users WHERE email = ? AND password = ?;";
        sqlite3_stmt* stmt;
        bool success = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                user.id = sqlite3_column_int(stmt, 0);
                user.email = std::string((const char*)sqlite3_column_text(stmt, 1));
                user.isAdmin = sqlite3_column_int(stmt, 2) != 0;
                success = true;
            }
        }
        sqlite3_finalize(stmt);
        return success;
    }

    // Search properties by name (returns basic struct without string conversion)
    std::vector<std::vector<std::string>> searchPropertiesByName(const std::string& searchTerm) {
        std::vector<std::vector<std::string>> results;
        if (!db) return results;

        std::string sql = "SELECT id, name, location, price, type, isAvailable, InfoNumber "
                          "FROM properties WHERE name LIKE ? OR location LIKE ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            std::string searchPattern = "%" + searchTerm + "%";
            sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, searchPattern.c_str(), -1, SQLITE_TRANSIENT);

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::vector<std::string> row;
                for (int i = 0; i < 7; i++) {
                    const char* text = (const char*)sqlite3_column_text(stmt, i);
                    row.push_back(text ? text : "");
                }
                results.push_back(row);
            }
        }
        sqlite3_finalize(stmt);
        return results;
    }

    // View all properties - returns prepared statement results
    sqlite3_stmt* getAllPropertiesStatement() {
        if (!db) return nullptr;

        std::string sql = "SELECT id, name, location, price, type, isAvailable, InfoNumber "
                          "FROM properties;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            return stmt;
        }
        return nullptr;
    }

    // View single property by ID - returns query results as vector
    std::vector<std::string> viewSingleProperty(int propertyId) {
        std::vector<std::string> result;
        if (!db) return result;

        std::string sql = "SELECT id, name, location, price, type, isAvailable, InfoNumber "
                          "FROM properties WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, propertyId);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                for (int i = 0; i < 7; i++) {
                    const char* text = (const char*)sqlite3_column_text(stmt, i);
                    result.push_back(text ? text : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        return result;
    }
};

#endif
