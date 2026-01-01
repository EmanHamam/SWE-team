# PropertyManager.h Documentation

## Overview
Handles property viewing and display functionality for regular users and guests. Provides interactive property browsing with detailed views and guest/user authentication checks.

## Dependencies
- `UserManager.h` - User authentication
- `DBManager.h` - Database operations
- `Property.h` - Property data structure
- `sqlite3.h` - Database types
- `<windows.h>`, `<conio.h>` - Console control

## External Global Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `isLoggedIn` | `bool` | Controls contact info visibility |
| `currentUserEmail` | `string` | Display logged-in user |

## Helper Functions

### `void setAttr(int i)`
**Purpose**: Sets console text color attribute.

**Parameters**:
- `i` - Color code (Windows console attribute)

### `void setXY(int x, int y)`
**Purpose**: Positions console cursor.

**Parameters**:
- `x`, `y` - Coordinates

## PropertyManager Class

### Public Methods

#### `void DisplayProperties(const vector<Property>& props, sqlite3* db, DBManager* dbManager)`
**Purpose**: Interactive table view of properties with navigation.

**Flow**:
1. Shows properties in table format
2. Allows navigation with arrow keys
3. Enter to view details
4. ESC to exit

#### `void ViewAllProperies(sqlite3* db, DBManager* dbManager)`
**Purpose**: Fetches and displays all available properties.

**Database Call**: `dbManager->getAllProperties()` (limit 10)

### Private Methods

#### `void showDetails(Property p, sqlite3* db, DBManager* dbMgr = nullptr)`
**Purpose**: Shows detailed property information with buy/rent options.

**Features**:
- Rent calculation (daily/monthly with discount)
- Availability status
- Contact info (locked for guests)
- Login prompt for guests

#### `void drawTableFrame()`
**Purpose**: Draws property table header.

## File Connections

| File | Connection |
|------|------------|
| `main.cpp` | Calls ViewAllProperies() |
| `DBManager.h` | Gets property data |
| `FilterationManager.h` | Uses DisplayProperties() for filtered results |
| `UserManager.h` | Login integration for guests |
