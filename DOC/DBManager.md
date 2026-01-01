# DBManager.h Documentation

## Overview
Core database management class that handles all SQLite operations for the Property Management System. Provides a centralized interface for database interactions including initialization, CRUD operations for properties and owners, and query execution.

## Dependencies
- `sqlite3.h` - SQLite C library
- `Property.h` - Property structure definition
- `<vector>` - For returning collections
- `<string>` - String operations

## Data Structures

### `struct User`
Represents a user in the system.

| Field | Type | Description |
|-------|------|-------------|
| `id` | `int` | Unique user identifier |
| `email` | `string` | User's email address |
| `isAdmin` | `bool` | Admin privilege flag |

## DBManager Class

### Private Members

| Member | Type | Purpose |
|--------|------|---------|
| `db` | `sqlite3*` | SQLite database connection pointer |
| `zErrMsg` | `char*` | Error message buffer |

---

## Constructor & Destructor

### `DBManager(const string& dbName)`
**Purpose**: Opens/creates SQLite database connection.

**Parameters**:
- `dbName` - Database file name (e.g., "test.db")

**Behavior**:
- Calls `sqlite3_open()` to establish connection
- Sets `db` to `nullptr` on failure
- Prints error message if connection fails

**Connected to**: Called in `main()` with "test.db"

---

### `~DBManager()`
**Purpose**: Destroys database connection on cleanup.

**Behavior**:
- Calls `sqlite3_close()` if connection exists
- Automatically called when `DBManager` instance goes out of scope

---

## Core Methods

### `sqlite3* getDB() const`
**Purpose**: Returns raw database pointer for direct SQL operations.

**Returns**: SQLite database pointer

**Connected to**: 
- Used by `PropertyManager`, `UserManager`, `SearchManager`
- Passed to various display and query functions

---

### `bool executeQuery(const string& sql, int (*callback)(void*, int, char**, char**) = nullptr, void* data = nullptr)`
**Purpose**: Executes arbitrary SQL queries with optional callback.

**Parameters**:
- `sql` - SQL statement to execute
- `callback` - Optional function called for each result row
- `data` - Optional user data passed to callback

**Returns**: `true` on success, `false` on error

**Error Handling**:
- Prints SQL error messages
- Frees error message buffer

**Connected to**: Used internally by `initializeDatabase()` and migration functions

---

### `void initializeDatabase()`
**Purpose**: Creates database schema and populates with initial data.

**Tables Created**:

1. **owners**
   ```sql
   CREATE TABLE IF NOT EXISTS owners (
       owner_id INTEGER PRIMARY KEY AUTOINCREMENT,
       name TEXT NOT NULL
   )
   ```

2. **properties**
   ```sql
   CREATE TABLE IF NOT EXISTS properties (
       id INTEGER PRIMARY KEY AUTOINCREMENT,
       name TEXT,
       location TEXT,
       price REAL,
       type TEXT,
       isAvailable INTEGER,
       InfoNumber TEXT,
       NoOfRooms INTEGER,
       NoOfBaths INTEGER,
       Area REAL,
       owner_id INTEGER,
       FOREIGN KEY (owner_id) REFERENCES owners(owner_id)
   )
   ```

3. **users**
   ```sql
   CREATE TABLE IF NOT EXISTS users (
       id INTEGER PRIMARY KEY AUTOINCREMENT,
       email TEXT UNIQUE,
       password TEXT,
       isAdmin INTEGER
   )
   ```

**Initial Data**:

- **7 Owners**: Omar Khaled, Mostafa Hendy, Eman Hammam, Mariam Magdy, Abdutalib, Omar Alsayeed, Mohamed Dawood

- **6 Properties**: Various villas and apartments in Cairo, Giza, and Alexandria

- **2 Default Users**:
  - Admin: `admin@system.com` / `admin123`
  - User: `user@system.com` / `user123`

**Connected to**: Called once by `main()` on application startup

---

## Property Retrieval Methods

### `vector<Property> getAllProperties()`
**Purpose**: Retrieves all properties with a limit of 10 for general viewing.

**Returns**: Vector of Property objects

**SQL Query**:
```sql
SELECT id, name, location, price, type, isAvailable, 
       InfoNumber, NoOfRooms, NoOfBaths, Area 
FROM properties 
LIMIT 10
```

**Connected to**: 
- Called by `PropertyManager.ViewAllProperies()`
- Used in guest/user property browsing

---

### `vector<Property> getAllPropertiesAdmin()`
**Purpose**: Retrieves all properties without limit for admin view.

**Returns**: Vector of Property objects including owner_id

**SQL Query**:
```sql
SELECT id, name, location, price, type, isAvailable, 
       InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id 
FROM properties
```

**Connected to**: 
- Called by `AdminManager.viewAllPropertiesAdmin()`
- Used in admin operations for full property listing

---

### `vector<Property> filterProperties(double maxPrice, const string& type, const string& location, int minRooms, int minBaths, double minArea)`
**Purpose**: Advanced property search with multiple filters.

**Parameters**:
- `maxPrice` - Maximum price filter (999999999 = no limit)
- `type` - "Buy" or "Rent" or empty for all
- `location` - Partial location match
- `minRooms` - Minimum number of rooms
- `minBaths` - Minimum number of bathrooms
- `minArea` - Minimum area in square meters

**Returns**: Vector of filtered properties (max 10 results)

**Dynamic SQL Building**:
- Base: `WHERE 1=1`
- Conditionally adds: `AND price <= ?`, `AND type = ?`, `AND LOWER(location) LIKE ?`, etc.
- Uses parameterized queries to prevent SQL injection

**Connected to**: 
- Called by `SearchManager.viewFilteredResults()`
- Used in `FilterationManager.h` for advanced search

---

### `bool getPropertyById(int propertyId, Property& prop)`
**Purpose**: Retrieves single property by ID.

**Parameters**:
- `propertyId` - Property ID to find
- `prop` - Reference to Property object to populate

**Returns**: `true` if found, `false` otherwise

**Connected to**: Used internally for property verification

---

### `bool getPropertyByIdWithOwner(int propertyId, Property& prop, int& ownerId)`
**Purpose**: Retrieves property with associated owner ID.

**Parameters**:
- `propertyId` - Property ID to find
- `prop` - Property object reference
- `ownerId` - Owner ID reference to populate

**Returns**: `true` if found, `false` otherwise

**Connected to**: 
- Called by `AdminManager.updateProperty()`
- Used when owner information is needed

---

### `bool propertyExists(int propertyId, string& propName)`
**Purpose**: Checks if property exists and returns its name.

**Parameters**:
- `propertyId` - Property ID to check
- `propName` - String reference to store property name

**Returns**: `true` if exists, `false` otherwise

**Connected to**: 
- Called by `AdminManager.deleteProperty()`
- Used for confirmation dialogs

---

## Owner Operations

### `vector<pair<int, string>> getAllOwners(int limit = 10)`
**Purpose**: Retrieves list of all owners with their IDs and names.

**Parameters**:
- `limit` - Maximum number of owners to return (default 10)

**Returns**: Vector of pairs (owner_id, name)

**SQL Query**:
```sql
SELECT owner_id, name 
FROM owners 
ORDER BY owner_id 
LIMIT ?
```

**Connected to**: 
- Called by `AdminManager.displayOwnersTable()`
- Used when adding/updating properties to show available owners

---

### `bool ownerExists(int ownerId)`
**Purpose**: Validates that an owner ID exists in the database.

**Parameters**:
- `ownerId` - Owner ID to verify

**Returns**: `true` if owner exists, `false` otherwise

**Connected to**: 
- Called by `AdminManager.addProperty()`
- Called by `AdminManager.updateProperty()`
- Ensures foreign key integrity

---

## CRUD Operations

### `bool addProperty(...)`
**Full Signature**:
```cpp
bool addProperty(const string& name, const string& location, double price,
                 const string& type, const string& infoNumber,
                 int rooms, int baths, double area, int ownerId, 
                 int& newPropertyId)
```

**Purpose**: Inserts new property into database.

**Parameters**:
- `name` - Property name
- `location` - Property location
- `price` - Property price
- `type` - "Buy" or "Rent"
- `infoNumber` - Contact phone number
- `rooms` - Number of rooms
- `baths` - Number of bathrooms
- `area` - Property area in m²
- `ownerId` - Owner's ID (foreign key)
- `newPropertyId` - Output parameter for new property's ID

**Returns**: `true` if insertion successful

**SQL Query**:
```sql
INSERT INTO properties (name, location, price, type, isAvailable, 
                        InfoNumber, NoOfRooms, NoOfBaths, Area, owner_id) 
VALUES (?, ?, ?, ?, 1, ?, ?, ?, ?, ?)
```

**Notes**:
- Sets `isAvailable = 1` (available) by default
- Uses `sqlite3_last_insert_rowid()` to get new ID

**Connected to**: Called by `AdminManager.addProperty()`

---

### `bool deleteProperty(int propertyId)`
**Purpose**: Removes property from database.

**Parameters**:
- `propertyId` - ID of property to delete

**Returns**: `true` if deletion successful

**SQL Query**:
```sql
DELETE FROM properties WHERE id = ?
```

**Connected to**: Called by `AdminManager.deleteProperty()` after user confirmation

---

### `bool updateProperty(...)`
**Full Signature**:
```cpp
bool updateProperty(int propertyId, const string& name, const string& location,
                    double price, const string& type, const string& infoNumber,
                    int rooms, int baths, double area, int ownerId)
```

**Purpose**: Updates existing property's information.

**Parameters**: All property fields including new owner ID

**Returns**: `true` if update successful

**SQL Query**:
```sql
UPDATE properties 
SET name=?, location=?, price=?, type=?, 
    InfoNumber=?, NoOfRooms=?, NoOfBaths=?, Area=?, owner_id=? 
WHERE id=?
```

**Connected to**: Called by `AdminManager.updateProperty()`

---

### `bool togglePropertyAvailability(int propertyId, string& propName, bool& newStatus)`
**Purpose**: Locks or unlocks a property (toggles availability).

**Parameters**:
- `propertyId` - Property to toggle
- `propName` - Output: property name
- `newStatus` - Output: new availability status after toggle

**Returns**: `true` if toggle successful

**Logic**:
1. Queries current `isAvailable` status
2. Toggles it (0→1 or 1→0)
3. Updates database
4. Returns new status

**SQL Queries**:
```sql
SELECT isAvailable, name FROM properties WHERE id = ?
UPDATE properties SET isAvailable = ? WHERE id = ?
```

**Connected to**: Called by `AdminManager.lockUnlockProperty()`

---

### `vector<Property> getPropertiesByOwner(int ownerId, string& ownerName)`
**Purpose**: Retrieves all properties belonging to specific owner.

**Parameters**:
- `ownerId` - Owner's ID
- `ownerName` - Output: owner's name from join

**Returns**: Vector of properties owned by that owner

**SQL Query**:
```sql
SELECT p.id, p.name, p.location, p.price, p.type, p.isAvailable, 
       p.InfoNumber, p.NoOfRooms, p.NoOfBaths, p.Area, o.name 
FROM properties p 
JOIN owners o ON p.owner_id = o.owner_id 
WHERE o.owner_id = ?
```

**Connected to**: Called by `AdminManager.viewPropertiesByOwner()`

---

## Migration Methods

### `void migrateAddRoomsBathsAndArea()`
**Purpose**: Adds new columns to existing database (schema migration).

**Columns Added**:
- `NoOfRooms INTEGER DEFAULT 0`
- `NoOfBaths INTEGER DEFAULT 0`
- `Area REAL DEFAULT 0`

**Note**: Currently commented out in `initializeDatabase()` - used for upgrading old databases

---

## Database Schema Diagram

```mermaid
erDiagram
    owners ||--o{ properties : owns
    users }o--|| properties : "views (no FK)"
    
    owners {
        int owner_id PK
        text name
    }
    
    properties {
        int id PK
        text name
        text location
        real price
        text type
        int isAvailable
        text InfoNumber
        int NoOfRooms
        int NoOfBaths
        real Area
        int owner_id FK
    }
    
    users {
        int id PK
        text email UNIQUE
        text password
        int isAdmin
    }
```

## File Connections

| Connected File | How It's Used |
|----------------|---------------|
| `main.cpp` | Creates DBManager instance, initializes database |
| `AdminManager.h` | All CRUD operations, owner queries |
| `PropertyManager.h` | getAllProperties() for viewing |
| `FilterationManager.h` | filterProperties() for search |
| `UserManager.cpp` | Direct SQL queries to `users` table |
| `Property.h` | Returns Property structs |
| `sqlite3.h` | All database operations |

## Key Design Patterns

1. **Centralized Database Access**: All SQL operations go through DBManager
2. **Prepared Statements**: Prevents SQL injection
3. **RAII Pattern**: Constructor opens DB, destructor closes it
4. **Separation of Concerns**: Database logic separated from UI logic
