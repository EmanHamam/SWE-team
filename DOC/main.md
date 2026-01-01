# main.cpp Documentation

## Overview
The main entry point of the Property Management System application. This file orchestrates the entire application flow, manages user sessions, and provides the interactive menu system.

## Dependencies
- `sqlite3.h` - Database operations
- `PropertyManager.h` - Property viewing and management
- `UserManager.h` - User authentication
- `DBManager.h` - Database management
- `FilterationManager.h` - Property searching and filtering
- `AdminManager.h` - Admin functionality
- `ConsoleUtils.h` - Console utility functions

## Global Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `isLoggedIn` | `bool` | Tracks whether a user is currently logged in |
| `currentUserId` | `int` | Stores the ID of the currently logged-in user |
| `currentUserEmail` | `string` | Stores the email of the current user |
| `isAdmin` | `bool` | Indicates if the current user has admin privileges |

## Functions

### `void drawHeader()`
**Purpose**: Displays the application's ASCII art header and title.

**Details**:
- Uses `textattr()` for colored text
- Uses `gotoxy()` for precise positioning
- Displays "REAL ESTATE" ASCII art
- Shows "PROPERTY MANAGEMENT SYSTEM" subtitle

**Connected to**: Called by `drawMenu()` when the menu needs to be redrawn.

---

### `void drawMenu(int selected, bool firstDraw)`
**Purpose**: Renders the main menu with different options based on user role.

**Parameters**:
- `selected` - Index of currently selected menu item
- `firstDraw` - Whether to clear screen and redraw header

**Menu Variations**:
1. **Guest Menu** (not logged in):
   - View Properties
   - Login
   - Signup
   - Search Properties
   - Exit

2. **User Menu** (logged in):
   - View Properties
   - Logout
   - Search Properties
   - Exit

3. **Admin Menu** (admin logged in):
   - Add Property
   - Delete Property
   - Update Property
   - Lock/Unlock Property
   - View All Properties
   - View Properties By Owner Id
   - Logout

**Connected to**: 
- Uses `drawHeader()` for the title
- Called repeatedly by `runMainMenu()` for menu navigation

---

### `bool executeMenuAction(int choice, sqlite3* db, DBManager* dbManager)`
**Purpose**: Executes the selected menu action based on user role and choice.

**Parameters**:
- `choice` - Index of selected menu option
- `db` - SQLite database pointer
- `dbManager` - Database manager instance

**Returns**: `true` to continue running, `false` to exit application

**Functionality**:
- **Guest Actions**:
  - Case 0: View properties via `PropertyManager.ViewAllProperies()`
  - Case 1: Login via `UserManager.login()`
  - Case 2: Signup via `UserManager.signup()`
  - Case 3: Search properties via `SearchManager.interactiveSearch()`
  - Case 4: Exit application

- **Admin Actions**:
  - Case 0: Add property via `AdminManager.addProperty()`
  - Case 1: Delete property via `AdminManager.deleteProperty()`
  - Case 2: Update property via `AdminManager.updateProperty()`
  - Case 3: Lock/Unlock via `AdminManager.lockUnlockProperty()`
  - Case 4: View all properties via `AdminManager.viewAllPropertiesAdmin()`
  - Case 5: View by owner via `AdminManager.viewPropertiesByOwner()`
  - Case 6: Logout (resets global variables)

- **Regular User Actions**:
  - Case 0: View properties
  - Case 1: Logout
  - Case 2: Search properties
  - Case 3: Exit

**Connected to**:
- Instantiates `PropertyManager`, `UserManager`, `SearchManager`, `AdminManager`
- Updates global session variables (`isLoggedIn`, `isAdmin`, etc.)
- Called by `runMainMenu()` when user presses Enter

---

### `void runMainMenu(sqlite3* db, DBManager* dbManager)`
**Purpose**: Main event loop that handles menu navigation and user input.

**Parameters**:
- `db` - SQLite database pointer
- `dbManager` - Database manager instance

**Input Handling**:
- **Arrow Up / 'w'**: Move selection up
- **Arrow Down / 's'**: Move selection down
- **Enter**: Execute selected action
- Hides cursor for cleaner UI

**Flow**:
1. Draws menu with current selection
2. Waits for keyboard input
3. Updates selection or executes action
4. Continues until user exits

**Connected to**:
- Calls `drawMenu()` for rendering
- Calls `executeMenuAction()` when user confirms selection

---

### `int main()`
**Purpose**: Application entry point that initializes the system.

**Flow**:
1. Creates `DBManager` instance with "test.db"
2. Checks if database opened successfully
3. Initializes database schema via `dbManager.initializeDatabase()`
4. Starts main menu loop via `runMainMenu()`

**Returns**: 0 on successful exit

**Connected to**:
- Uses `DBManager` class from `DBManager.h`
- Passes database references to `runMainMenu()`

## System Architecture

```
main.cpp
│
├─ Initializes DBManager
│
├─ Calls runMainMenu()
│   │
│   ├─ Displays drawMenu()
│   │   └─ Shows drawHeader()
│   │
│   └─ Executes executeMenuAction()
│       ├─ PropertyManager (view, display)
│       ├─ UserManager (login, signup)
│       ├─ SearchManager (filter search)
│       └─ AdminManager (CRUD operations)
```

## File Connections

| Connected File | Purpose of Connection |
|----------------|----------------------|
| `DBManager.h` | Database initialization and queries |
| `PropertyManager.h` | Viewing and displaying properties |
| `UserManager.h` | User authentication (login/signup) |
| `FilterationManager.h` | Advanced property search |
| `AdminManager.h` | Admin CRUD operations |
| `ConsoleUtils.h` | Console formatting utilities |
| `sqlite3.h` | Direct database access |
