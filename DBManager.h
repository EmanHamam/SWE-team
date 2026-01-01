#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <iostream>
#include <string>
#include <vector>
#include "sqlite3.h"
#include "Property.h"

using namespace std;

struct User
{
    int id;
    string email;
    bool isAdmin;
};

class DBManager
{
private:
    sqlite3 *db;
    char *zErrMsg;

public:
    DBManager(const string &dbName) : db(nullptr), zErrMsg(nullptr)
    {
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc)
        {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
            db = nullptr;
        }
    }

    ~DBManager()
    {
        if (db)
        {
            sqlite3_close(db);
        }
    }

    sqlite3 *getDB() const
    {
        return db;
    }

    bool executeQuery(const string &sql,
                      int (*callback)(void *, int, char **, char **) = nullptr,
                      void *data = nullptr)
    {

        if (!db)
            return false;

        int rc = sqlite3_exec(db, sql.c_str(), callback, data, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error: " << zErrMsg << endl;
            sqlite3_free(zErrMsg);
            return false;
        }
        return true;
    }

    void initializeDatabase()
    {
        string sql =
            "CREATE TABLE IF NOT EXISTS owners ("
            "owner_id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT NOT NULL);"

            "CREATE TABLE IF NOT EXISTS properties ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT, "
            "location TEXT, "
            "price REAL, "
            "type TEXT, "
            "isAvailable INTEGER, "
            "InfoNumber TEXT, "
            "NoOfRooms INTEGER, "
            "NoOfBaths INTEGER, "
            "Area REAL);"
            "owner_id INTEGER, "
            "FOREIGN KEY (owner_id) REFERENCES owners(owner_id));"

            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "email TEXT UNIQUE, "
            "password TEXT, "
            "isAdmin INTEGER);"

            "CREATE TABLE IF NOT EXISTS appointments ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL, "
            "property_id INTEGER NOT NULL, "
            "appointment_date TEXT NOT NULL, "
            "status TEXT DEFAULT 'pending', "
            "notes TEXT, "
            "FOREIGN KEY (user_id) REFERENCES users(id), "
            "FOREIGN KEY (property_id) REFERENCES properties(id));"

            "INSERT OR IGNORE INTO properties "
            "(name, location, price, type, isAvailable, InfoNumber, NoOfRooms, NoOfBaths, Area) VALUES "
            "('Ocean View Villa', 'Malibu', 1250000.0, 'Buy', 1, '555-0101', 5, 4, 3500.0),"
            "('Downtown Apt', 'New York', 3500.0, 'Rent', 1, '555-0202', 2, 1, 850.0),"
            "('Mountain Cabin', 'Aspen', 450000.0, 'Buy', 0, '555-0303', 3, 2, 1200.0);"

            // Insert owners
            "INSERT OR IGNORE INTO owners (name) VALUES "
            "('John Doe'),"
            "('Alice Smith'),"
            "('Michael Brown');"

            // Insert properties with owner_id
            "INSERT OR IGNORE INTO properties (name, location, price, type, isAvailable, InfoNumber, owner_id) VALUES "
            "('Ocean View Villa', 'Malibu', 1250000.0, 'Buy', 1, '555-0101', 1),"
            "('Downtown Apt', 'New York', 3500.0, 'Rent', 1, '555-0202', 2),"
            "('Mountain Cabin', 'Aspen', 450000.0, 'Buy', 0, '555-0303', 3);"

            // Insert default users
            "INSERT OR IGNORE INTO users (email, password, isAdmin) VALUES "
            "('admin@system.com', 'admin123', 1),"
            "('user@system.com', 'user123', 0);";

        executeQuery(sql);
        // Migration for old databases
        migrateAddRoomsBathsAndArea();
    }

    // ---------------- MIGRATION ----------------
    void migrateAddRoomsBathsAndArea()
    {
        executeQuery("ALTER TABLE properties ADD COLUMN NoOfRooms INTEGER DEFAULT 0;");
        executeQuery("ALTER TABLE properties ADD COLUMN NoOfBaths INTEGER DEFAULT 0;");
        executeQuery("ALTER TABLE properties ADD COLUMN Area REAL DEFAULT 0;");
    }

    // ---------------- AUTHENTICATION ----------------
    bool validateUser(const string &email,
                      const string &password,
                      int &userId,
                      bool &isAdmin)
    {

        string sql =
            "SELECT id, isAdmin FROM users WHERE email = ? AND password = ?;";

        sqlite3_stmt *stmt;
        bool success = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                userId = sqlite3_column_int(stmt, 0);
                isAdmin = sqlite3_column_int(stmt, 1) != 0;
                success = true;
            }
        }
        sqlite3_finalize(stmt);
        return success;
    }

    bool authenticate(const string &email,
                      const string &password,
                      User &user)
    {

        if (!db)
            return false;

        string sql = "SELECT id, email, isAdmin FROM users WHERE email = ? AND password = ?;";
        sqlite3_stmt *stmt;
        bool success = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                user.id = sqlite3_column_int(stmt, 0);
                user.email = (const char *)sqlite3_column_text(stmt, 1);
                user.isAdmin = sqlite3_column_int(stmt, 2) != 0;
                success = true;
            }
        }
        sqlite3_finalize(stmt);
        return success;
    }

    // ---------------- SEARCH PROPERTIES ----------------
    vector<vector<string>>
    searchPropertiesByName(const string &searchTerm)
    {

        vector<vector<string>> results;
        if (!db)
            return results;

        string sql =
            "SELECT id, name, location, price, type, isAvailable, "
            "InfoNumber, NoOfRooms, NoOfBaths, Area "
            "FROM properties WHERE name LIKE ? OR location LIKE ?;";

        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
        {

            string pattern = "%" + searchTerm + "%";
            sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                vector<string> row;
                for (int i = 0; i < 10; i++)
                { // 10 columns now
                    const char *text =
                        (const char *)sqlite3_column_text(stmt, i);
                    row.push_back(text ? text : "");
                }
                results.push_back(row);
            }
        }
        sqlite3_finalize(stmt);
        return results;
    }

    // ---------------- VIEW ALL PROPERTIES ----------------
    sqlite3_stmt *getAllPropertiesStatement()
    {
        if (!db)
            return nullptr;

        string sql =
            "SELECT id, name, location, price, type, isAvailable, "
            "InfoNumber, NoOfRooms, NoOfBaths, Area FROM properties;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
            return stmt;

        return nullptr;
    }

    // ---------------- VIEW SINGLE PROPERTY ----------------
    vector<string> viewSingleProperty(int propertyId)
    {

        vector<string> result;
        if (!db)
            return result;

        string sql =
            "SELECT id, name, location, price, type, isAvailable, "
            "InfoNumber, NoOfRooms, NoOfBaths, Area "
            "FROM properties WHERE id = ?;";

        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, propertyId);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                for (int i = 0; i < 10; i++)
                { // 10 columns
                    const char *text =
                        (const char *)sqlite3_column_text(stmt, i);
                    result.push_back(text ? text : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        return result;
    }

    // ---------------- GET ALL PROPERTIES ----------------
    vector<Property> getAllProperties()
    {
        vector<Property> list;
        if (!db)
            return list;

        string sql =
            "SELECT id, name, location, price, type, isAvailable, "
            "InfoNumber, NoOfRooms, NoOfBaths, Area FROM properties Limit 10;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                Property p;
                p.id = sqlite3_column_int(stmt, 0);
                p.name = (const char *)sqlite3_column_text(stmt, 1);
                p.location = (const char *)sqlite3_column_text(stmt, 2);
                p.price = sqlite3_column_double(stmt, 3);
                p.type = (const char *)sqlite3_column_text(stmt, 4);
                p.available = sqlite3_column_int(stmt, 5);
                p.infoNumber = (const char *)sqlite3_column_text(stmt, 6);
                p.noOfRooms = sqlite3_column_int(stmt, 7);
                p.noOfBaths = sqlite3_column_int(stmt, 8);
                p.area = sqlite3_column_double(stmt, 9);
                list.push_back(p);
            }
        }
        sqlite3_finalize(stmt);
        return list;
    }

    // ---------------- FILTER PROPERTIES ----------------
    vector<Property> filterProperties(double maxPrice, const string &type, const string &location, int minRooms, int minBaths, double minArea)
    {
        vector<Property> list;
        if (!db)
            return list;

        string sql =
            "SELECT id, name, location, price, type, isAvailable, "
            "InfoNumber, NoOfRooms, NoOfBaths, Area FROM properties WHERE 1=1";

        if (maxPrice < 999999999)
            sql += " AND price <= ?";
        if (!type.empty())
            sql += " AND type = ?";
        if (!location.empty())
            sql += " AND LOWER(location) LIKE ?";
        if (minRooms > 0)
            sql += " AND NoOfRooms >= ?";
        if (minBaths > 0)
            sql += " AND NoOfBaths >= ?";
        if (minArea > 0)
            sql += " AND Area >= ?";
        sql += " LIMIT 10";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
        {
            int paramIndex = 1;
            if (maxPrice < 999999999)
                sqlite3_bind_double(stmt, paramIndex++, maxPrice);
            if (!type.empty())
                sqlite3_bind_text(stmt, paramIndex++, type.c_str(), -1, SQLITE_STATIC);
            if (!location.empty())
            {
                string pattern = "%" + location + "%";
                sqlite3_bind_text(stmt, paramIndex++, pattern.c_str(), -1, SQLITE_TRANSIENT);
            }
            if (minRooms > 0)
                sqlite3_bind_int(stmt, paramIndex++, minRooms);
            if (minBaths > 0)
                sqlite3_bind_int(stmt, paramIndex++, minBaths);
            if (minArea > 0)
                sqlite3_bind_double(stmt, paramIndex++, minArea);

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                Property p;
                p.id = sqlite3_column_int(stmt, 0);
                p.name = (const char *)sqlite3_column_text(stmt, 1);
                p.location = (const char *)sqlite3_column_text(stmt, 2);
                p.price = sqlite3_column_double(stmt, 3);
                p.type = (const char *)sqlite3_column_text(stmt, 4);
                p.available = sqlite3_column_int(stmt, 5);
                p.infoNumber = (const char *)sqlite3_column_text(stmt, 6);
                p.noOfRooms = sqlite3_column_int(stmt, 7);
                p.noOfBaths = sqlite3_column_int(stmt, 8);
                p.area = sqlite3_column_double(stmt, 9);
                list.push_back(p);
            }
        }
        sqlite3_finalize(stmt);
        return list;
    }
};

#endif
