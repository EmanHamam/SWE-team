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
            "Area REAL, "
            "owner_id INTEGER, "
            "FOREIGN KEY (owner_id) REFERENCES owners(owner_id));"

            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "email TEXT UNIQUE, "
            "password TEXT, "
            "isAdmin INTEGER);"

            // Insert owners
            "INSERT OR IGNORE INTO owners (owner_id, name) VALUES "
            "(1, 'Omar Khaled'),"
            "(2, 'Mostafa Hendy'),"
            "(3, 'Eman Hammam'),"
            "(4, 'Mariam Magdy'),"
            "(5, 'Abdutalib'),"
            "(6, 'Omar Alsayeed'),"
            "(7, 'Mohamed Dawood');"

            // Insert properties with owner_id
            "INSERT OR IGNORE INTO properties (name, location, price, type, isAvailable, InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id) VALUES "
            "('giza villa', 'giza', 5000.0, 'Buy', 1, '0123456789', 5, 4, 350.0, 1),"
            "('cairo apartment', 'cairo', 2000.0, 'Rent', 1, '0152468486', 3, 2, 120.0, 2),"
            "('alexandria villa', 'alexandria', 3000.0, 'Buy', 1, '0112233445', 4, 3, 200.0, 4),"
            "('giza apartment', 'giza', 2500.0, 'Rent', 1, '0198765432', 2, 1, 100.0, 5),"
            "('cairo villa', 'cairo', 6000.0, 'Buy', 1, '0176543210', 5, 4, 400.0, 6),"
            "('alexandria apartment', 'alexandria', 2200.0, 'Rent', 1, '0135792468', 3, 2, 150.0, 7);"

            // Insert default users
            "INSERT OR IGNORE INTO users (email, password, isAdmin) VALUES "
            "('admin@system.com', 'admin123', 1),"
            "('user@system.com', 'user123', 0);";

        executeQuery(sql);
        // Migration for old databases
        // migrateAddRoomsBathsAndArea();
    }

    // ---------------- MIGRATION ----------------
    void migrateAddRoomsBathsAndArea()
    {
        executeQuery("ALTER TABLE properties ADD COLUMN NoOfRooms INTEGER DEFAULT 0;");
        executeQuery("ALTER TABLE properties ADD COLUMN NoOfBaths INTEGER DEFAULT 0;");
        executeQuery("ALTER TABLE properties ADD COLUMN Area REAL DEFAULT 0;");
    }

    // ---------------- AUTHENTICATION ----------------
    // Note: Authentication is now handled by UserManager class

    // ---------------- PROPERTY OPERATIONS ----------------
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
            "InfoNumber, NoOfRooms, NoOfBaths, Area FROM properties where 1=1";

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

    // ================= OWNER OPERATIONS =================

    // Get all owners (id and name)
    vector<pair<int, string>> getAllOwners(int limit = 10)
    {
        vector<pair<int, string>> owners;
        if (!db) return owners;

        string sql = "SELECT owner_id, name FROM owners ORDER BY owner_id LIMIT " + to_string(limit) + ";";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                int id = sqlite3_column_int(stmt, 0);
                string name = (const char *)sqlite3_column_text(stmt, 1);
                owners.push_back(make_pair(id, name));
            }
        }
        sqlite3_finalize(stmt);
        return owners;
    }

    // Check if owner exists
    bool ownerExists(int ownerId)
    {
        if (!db) return false;

        string sql = "SELECT owner_id FROM owners WHERE owner_id = ?;";
        sqlite3_stmt *stmt;
        bool exists = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, ownerId);
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                exists = true;
            }
        }
        sqlite3_finalize(stmt);
        return exists;
    }

    // ================= PROPERTY CRUD OPERATIONS =================

    // Get all properties (for admin listing)
    vector<Property> getAllPropertiesAdmin()
    {
        vector<Property> list;
        if (!db) return list;

        string sql = "SELECT id, name, location, price, type, isAvailable, "
                     "InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id FROM properties;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
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

    // Get property by ID
    bool getPropertyById(int propertyId, Property &prop)
    {
        if (!db) return false;

        string sql = "SELECT id, name, location, price, type, isAvailable, "
                     "InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id FROM properties WHERE id = ?;";
        sqlite3_stmt *stmt;
        bool found = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, propertyId);
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                prop.id = sqlite3_column_int(stmt, 0);
                prop.name = (const char *)sqlite3_column_text(stmt, 1);
                prop.location = (const char *)sqlite3_column_text(stmt, 2);
                prop.price = sqlite3_column_double(stmt, 3);
                prop.type = (const char *)sqlite3_column_text(stmt, 4);
                prop.available = sqlite3_column_int(stmt, 5);
                prop.infoNumber = (const char *)sqlite3_column_text(stmt, 6);
                prop.noOfRooms = sqlite3_column_int(stmt, 7);
                prop.noOfBaths = sqlite3_column_int(stmt, 8);
                prop.area = sqlite3_column_double(stmt, 9);
                found = true;
            }
        }
        sqlite3_finalize(stmt);
        return found;
    }

    // Get property by ID with owner_id
    bool getPropertyByIdWithOwner(int propertyId, Property &prop, int &ownerId)
    {
        if (!db) return false;

        string sql = "SELECT id, name, location, price, type, isAvailable, "
                     "InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id FROM properties WHERE id = ?;";
        sqlite3_stmt *stmt;
        bool found = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, propertyId);
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                prop.id = sqlite3_column_int(stmt, 0);
                prop.name = (const char *)sqlite3_column_text(stmt, 1);
                prop.location = (const char *)sqlite3_column_text(stmt, 2);
                prop.price = sqlite3_column_double(stmt, 3);
                prop.type = (const char *)sqlite3_column_text(stmt, 4);
                prop.available = sqlite3_column_int(stmt, 5);
                prop.infoNumber = (const char *)sqlite3_column_text(stmt, 6);
                prop.noOfRooms = sqlite3_column_int(stmt, 7);
                prop.noOfBaths = sqlite3_column_int(stmt, 8);
                prop.area = sqlite3_column_double(stmt, 9);
                ownerId = sqlite3_column_int(stmt, 10);
                found = true;
            }
        }
        sqlite3_finalize(stmt);
        return found;
    }

    // Check if property exists (returns name if found)
    bool propertyExists(int propertyId, string &propName)
    {
        if (!db) return false;

        string sql = "SELECT name FROM properties WHERE id = ?;";
        sqlite3_stmt *stmt;
        bool exists = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, propertyId);
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                propName = (const char *)sqlite3_column_text(stmt, 0);
                exists = true;
            }
        }
        sqlite3_finalize(stmt);
        return exists;
    }

    // Add new property
    bool addProperty(const string &name, const string &location, double price,
                     const string &type, const string &infoNumber,
                     int rooms, int baths, double area, int ownerId, int &newPropertyId)
    {
        if (!db) return false;

        string sql = "INSERT INTO properties (name, location, price, type, isAvailable, "
                     "InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id) VALUES (?, ?, ?, ?, 1, ?, ?, ?, ?, ?);";
        sqlite3_stmt *stmt;
        bool success = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, location.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 3, price);
            sqlite3_bind_text(stmt, 4, type.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, infoNumber.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 6, rooms);
            sqlite3_bind_int(stmt, 7, baths);
            sqlite3_bind_double(stmt, 8, area);
            sqlite3_bind_int(stmt, 9, ownerId);

            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                newPropertyId = (int)sqlite3_last_insert_rowid(db);
                success = true;
            }
        }
        sqlite3_finalize(stmt);
        return success;
    }

    // Delete property
    bool deleteProperty(int propertyId)
    {
        if (!db) return false;

        string sql = "DELETE FROM properties WHERE id = ?;";
        sqlite3_stmt *stmt;
        bool success = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, propertyId);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                success = true;
            }
        }
        sqlite3_finalize(stmt);
        return success;
    }

    // Update property
    bool updateProperty(int propertyId, const string &name, const string &location,
                        double price, const string &type, const string &infoNumber,
                        int rooms, int baths, double area, int ownerId)
    {
        if (!db) return false;

        string sql = "UPDATE properties SET name=?, location=?, price=?, type=?, "
                     "InfoNumber=?, NoOfRooms=?, NoOfBaths=?, Area=?, owner_id=? WHERE id=?;";
        sqlite3_stmt *stmt;
        bool success = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, location.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 3, price);
            sqlite3_bind_text(stmt, 4, type.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, infoNumber.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 6, rooms);
            sqlite3_bind_int(stmt, 7, baths);
            sqlite3_bind_double(stmt, 8, area);
            sqlite3_bind_int(stmt, 9, ownerId);
            sqlite3_bind_int(stmt, 10, propertyId);

            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                success = true;
            }
        }
        sqlite3_finalize(stmt);
        return success;
    }

    // Toggle property availability (lock/unlock)
    bool togglePropertyAvailability(int propertyId, string &propName, bool &newStatus)
    {
        if (!db) return false;

        // First get current status and name
        string sql = "SELECT isAvailable, name FROM properties WHERE id = ?;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, propertyId);
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                int currentStatus = sqlite3_column_int(stmt, 0);
                propName = (const char *)sqlite3_column_text(stmt, 1);
                sqlite3_finalize(stmt);

                // Toggle status
                newStatus = (currentStatus == 0);
                string updateSql = "UPDATE properties SET isAvailable = ? WHERE id = ?;";
                sqlite3_stmt *updateStmt;

                if (sqlite3_prepare_v2(db, updateSql.c_str(), -1, &updateStmt, nullptr) == SQLITE_OK)
                {
                    sqlite3_bind_int(updateStmt, 1, newStatus ? 1 : 0);
                    sqlite3_bind_int(updateStmt, 2, propertyId);
                    bool success = (sqlite3_step(updateStmt) == SQLITE_DONE);
                    sqlite3_finalize(updateStmt);
                    return success;
                }
            }
        }
        sqlite3_finalize(stmt);
        return false;
    }

    // Get properties by owner ID
    vector<Property> getPropertiesByOwner(int ownerId, string &ownerName)
    {
        vector<Property> list;
        if (!db) return list;

        string sql = "SELECT p.id, p.name, p.location, p.price, p.type, p.isAvailable, "
                     "p.InfoNumber, p.NoOfRooms, p.NoOfBaths, p.Area, o.name "
                     "FROM properties p "
                     "JOIN owners o ON p.owner_id = o.owner_id "
                     "WHERE o.owner_id = ?;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, ownerId);
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
                ownerName = (const char *)sqlite3_column_text(stmt, 10);
                list.push_back(p);
            }
        }
        sqlite3_finalize(stmt);
        return list;
    }
};

#endif
