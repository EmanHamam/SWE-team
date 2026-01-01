# AdminManager.h Documentation

## Overview
Provides administrative functionality for managing properties in the system. Contains methods to add, delete, update, lock/unlock properties, and view properties with various filters. Uses `DBManager` for all database operations.

## Class Structure
Header-only implementation - entire class defined in `AdminManager.h`

## Dependencies
- `DBManager.h` - Database operations
- `Property.h` - Property data structure
- `sqlite3.h` - Database types
- `ConsoleUtils.h` - Console utilities (`gotoxy()`, `textattr()`, `multiLineEditor()`)
- `<iomanip>` - Output formatting
- `<conio.h>` - Keyboard input (`_getch()`)

## External Global Variables

| Variable | Type | Used For |
|----------|------|----------|
| `isLoggedIn` | `bool` | Admin authentication check |
| `currentUserEmail` | `string` | Display current admin email |

## AdminManager Class

### Private Methods

#### `string trim(const char* s)`
**Purpose**: Removes leading and trailing whitespace from strings.

**Parameters**:
- `s` - C-string to trim

**Returns**: Trimmed string

**Logic**:
1. Finds first non-space character
2. Removes trailing spaces
3. Returns cleaned string

**Connected to**: Used internally to clean user input from `multiLineEditor()`

---

#### `void displayOwnersTable(DBManager* dbManager, int startX, int startY)`
**Purpose**: Displays formatted table of available property owners.

**Parameters**:
- `dbManager` - Database manager instance
- `startX` - X coordinate for table position
- `startY` - Y coordinate for table position

**Table Format**:
```
 ID   | Owner Name            
------+------------------------
 1    | Omar Khaled
 2    | Mostafa Hendy
 ...
```

**Data Source**: Calls `dbManager->getAllOwners(10)` to retrieve up to 10 owners

**Connected to**: 
- Called by `addProperty()` to show available owners
- Called by `updateProperty()` to display owner options
- Called by `viewPropertiesByOwner()` for owner selection

---

#### `void displayPropertiesTable(const vector<Property>& props, bool showOwnerId = false)`
**Purpose**: Renders property data in formatted table.

**Parameters**:
- `props` - Vector of properties to display
- `showOwnerId` - Whether to include owner ID column

**Two Display Modes**:

1. **Standard Mode** (showOwnerId = false):
   - Columns: ID, NAME, LOCATION, PRICE, TYPE, AVAILABLE

2. **Detailed Mode** (showOwnerId = true):
   - Columns: ID, NAME, LOCATION, PRICE, TYPE, CONTACT, ROOMS, BATHS, AREA, OWNER ID, AVAIL

**Formatting**:
- Uses `setw()` for column alignment
- Truncates long strings with `substr()`
- Formats prices with fixed precision

**Connected to**: 
- Called by `deleteProperty()` to show property list
- Used internally for property display

---

### Public Methods

#### `void addProperty(DBManager* dbManager)`
**Purpose**: Interactive form to add new property to the system.

**Parameters**:
- `dbManager` - Database manager instance

**UI Layout**:
```
==================================================
|             ADD NEW PROPERTY                    |
|                                                 |
| Property Name:    [input field]                |
| Location:         [input field]                |
| Price:            [input field]                |
| Type (Buy/Rent):  [input field]                |
| Contact Number:   [input field]                |
| Number of Rooms:  [input field]                |
| Number of Baths:  [input field]                |
| Area:             [input field]                |
| Owner ID:         [input field]                |
==================================================

[Right side shows owner table]
```

**Input Fields** (9 fields via `multiLineEditor()`):
1. Property Name (text)
2. Location (text)
3. Price (numeric)
4. Type (text: Buy/Rent)
5. Contact Number (text)
6. Number of Rooms (numeric)
7. Number of Bathrooms (numeric)
8. Area (numeric)
9. Owner ID (numeric)

**Validation**:
- All fields required
- Type must be "Buy" or "Rent" (case-insensitive)
- Type normalized to proper case
- Owner ID must exist (checked via `dbManager->ownerExists()`)

**Database Operation**:
- Calls `dbManager->addProperty(...)`
- Displays new property ID on success

**Connected to**: 
- Called by `executeMenuAction()` in `main.cpp` (Admin menu option 0)
- Uses `DBManager::addProperty()`
- Uses `DBManager::ownerExists()`
- Uses `DBManager::getAllOwners()`

---

#### `void deleteProperty(DBManager* dbManager)`
**Purpose**: Allows admin to delete a property after confirmation.

**Flow**:
1. Displays all properties in table format
2. Prompts for property ID
3. Verifies property exists
4. Shows confirmation dialog with property name
5. Deletes if confirmed

**User Interaction**:
```
Enter Property ID to delete: 5
Are you sure you want to delete property: cairo apartment? (Y/N):
```

**Validation**:
- Checks if property exists via `dbManager->propertyExists()`
- Requires Y/y confirmation to proceed

**Database Operation**:
- Calls `dbManager->deleteProperty(id)` if confirmed

**Connected to**: 
- Called by `executeMenuAction()` in `main.cpp` (Admin menu option 1)
- Uses `DBManager::getAllPropertiesAdmin()`
- Uses `DBManager::propertyExists()`
- Uses `DBManager::deleteProperty()`

---

#### `void updateProperty(DBManager* dbManager)`
**Purpose**: Interactive form to update existing property information.

**Flow**:
1. Prompts for property ID
2. Retrieves current property data
3. Displays update form with current values
4. Allows editing (blank = keep current value)
5. Validates and saves changes

**UI Layout**:
```
==================================================
|             UPDATE PROPERTY                     |
| Property ID: 3                                  |
|                                                 |
| Name:         [current: old value]             |
| Location:     [current: old value]             |
| Price:        [current: old value]             |
| Type:         [current: old value]             |
| Contact:      [current: old value]             |
| Rooms:        [current: old value]             |
| Baths:        [current: old value]             |
| Area:         [current: old value]             |
| Owner ID:     [current: old value]             |
|                                                 |
| Blank = keep: [property name preview]          |
==================================================

[Right side shows owner table]
```

**Input Behavior**:
- Empty input keeps existing value
- New input replaces existing value
- Uses `strlen()` to detect empty fields

**Validation**:
- Type must be "Buy" or "Rent" if changed
- Owner ID must exist if changed

**Database Operations**:
- Calls `dbManager->getPropertyByIdWithOwner()` to load current data
- Calls `dbManager->updateProperty()` to save changes
- Calls `dbManager->ownerExists()` for validation

**Connected to**: 
- Called by `executeMenuAction()` in `main.cpp` (Admin menu option 2)
- Uses multiple `DBManager` methods

---

#### `void lockUnlockProperty(DBManager* dbManager)`
**Purpose**: Toggles property availability status (locks or unlocks).

**Flow**:
1. Prompts for property ID
2. Toggles `isAvailable` field in database
3. Displays result (locked/unlocked)

**User Interaction**:
```
Enter Property ID to lock/unlock: 3
Property 'cairo apartment' unlocked successfully.
```

**Database Operation**:
- Calls `dbManager->togglePropertyAvailability()`
- Automatically flips status (0→1 or 1→0)

**Use Case**:
- Admin can temporarily disable property from being shown/rented
- Useful for maintenance or removal from market

**Connected to**: 
- Called by `executeMenuAction()` in `main.cpp` (Admin menu option 3)
- Uses `DBManager::togglePropertyAvailability()`

---

#### `void viewAllPropertiesAdmin(DBManager* dbManager)`
**Purpose**: Displays complete list of all properties (no pagination limit).

**Display Format**:
```
====== ALL PROPERTIES (ADMIN VIEW) ======

 ID  | NAME              | LOCATION        | PRICE       | TYPE  | CONTACT      | ROOMS | BATHS | AREA    | AVAIL
------------------------------------------------------------------------------------------------
 1   | giza villa        | giza            | $5000       | Buy   | 0123456789   | 5     | 4     | 350.0   | Yes
 2   | cairo apartment   | cairo           | $2000       | Rent  | 0152468486   | 3     | 2     | 120.0   | Yes
...
```

**Features**:
- Shows all properties without limit
- Includes all property details
- Shows availability status

**Database Operation**:
- Calls `dbManager->getAllPropertiesAdmin()`

**Connected to**: 
- Called by `executeMenuAction()` in `main.cpp` (Admin menu option 4)

---

#### `void viewPropertiesByOwner(DBManager* dbManager)`
**Purpose**: Displays all properties owned by a specific owner.

**Flow**:
1. Shows owner table
2. Prompts for owner ID
3. Retrieves and displays properties for that owner

**Display**:
```
Available Owners:
 ID   | Owner Name            
------+------------------------
 1    | Omar Khaled
 2    | Mostafa Hendy
...

Enter Owner ID: 2

====== Properties for Owner: Mostafa Hendy (ID: 2) ======
 ID  | NAME              | LOCATION        | PRICE       | TYPE  | CONTACT      | ROOMS | BATHS | AREA    | AVAIL
...
```

**Database Operations**:
- Calls `dbManager->getAllOwners()` to display options
- Calls `dbManager->getPropertiesByOwner()` to fetch results

**Connected to**: 
- Called by `executeMenuAction()` in `main.cpp` (Admin menu option 5)

---

## Data Flow Diagrams

### Add Property Flow
```
Admin selects "Add Property"
    │
    ├─ Display form with owner table
    │
    ├─ Collect 9 input fields via multiLineEditor()
    │
    ├─ Validate all fields
    │   ├─ Check if empty
    │   ├─ Validate type (Buy/Rent)
    │   └─ Verify owner exists
    │
    ├─ Call dbManager->addProperty()
    │
    └─ Display success with new property ID
```

### Update Property Flow
```
Admin selects "Update Property"
    │
    ├─ Enter property ID
    │
    ├─ Fetch current property data
    │
    ├─ Display form with current values
    │
    ├─ Edit fields (blank = keep old value)
    │
    ├─ Validate changes
    │
    ├─ Call dbManager->updateProperty()
    │
    └─ Display success message
```

### Delete Property Flow
```
Admin selects "Delete Property"
    │
    ├─ Show all properties
    │
    ├─ Enter property ID
    │
    ├─ Verify property exists
    │
    ├─ Confirm deletion (Y/N)
    │
    ├─ If Yes: dbManager->deleteProperty()
    │
    └─ Display result
```

## UI Helper Functions

### Character Ranges for multiLineEditor
```cpp
char sr[9] = {32, 32, 48, 32, 32, 48, 48, 48, 48};
char er[9] = {126, 126, 57, 126, 126, 57, 57, 57, 57};
```

**Translation**:
- Fields 0, 1, 3, 4: ASCII 32-126 (all printable chars)
- Field 2 (price): ASCII 48-57 (digits 0-9)
- Fields 5, 6, 7, 8 (rooms, baths, area, owner): Digits only

## File Connections

| Connected File | Purpose |
|----------------|---------|
| `main.cpp` | Calls all public methods from admin menu |
| `DBManager.h` | All database operations |
| `Property.h` | Property struct for data handling |
| `ConsoleUtils.h` | UI rendering (`gotoxy`, `textattr`, `multiLineEditor`) |
| `sqlite3.h` | Database types |

## Admin Menu Integration

| Menu Option | Function Called |
|-------------|-----------------|
| 0 - Add Property | `addProperty()` |
| 1 - Delete Property | `deleteProperty()` |
| 2 - Update Property | `updateProperty()` |
| 3 - Lock/Unlock Property | `lockUnlockProperty()` |
| 4 - View All Properties | `viewAllPropertiesAdmin()` |
| 5 - View Properties By Owner | `viewPropertiesByOwner()` |
| 6 - Logout | (handled in main.cpp) |

## Key Features

✅ **Complete CRUD Operations**: Create, Read, Update, Delete properties  
✅ **Owner Management**: View and validate property owners  
✅ **Input Validation**: Comprehensive checks for data integrity  
✅ **Rich UI**: Formatted tables and forms with console graphics  
✅ **Confirmation Dialogs**: Prevents accidental deletions  
✅ **Real-time Feedback**: Immediate success/error messages
