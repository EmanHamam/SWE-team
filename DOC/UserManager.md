# UserManager.cpp and UserManager.h Documentation

## Overview
Handles user authentication including login and signup functionality with an interactive console-based UI. Provides input validation for emails and passwords.

## Files
- `UserManager.h` - Class declaration
- `UserManager.cpp` - Implementation

## Dependencies
- `sqlite3.h` - Database operations
- `ConsoleUtils.h` - Console positioning and formatting
- `<regex>` - Email and password validation
- `<conio.h>` - Keyboard input handling
- `<windows.h>` - Console attributes

## External Global Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `isLoggedIn` | `bool` | Set to `true` when login succeeds |
| `currentUserId` | `int` | Stores authenticated user's ID |
| `currentUserEmail` | `string` | Stores authenticated user's email |

## Helper Functions

### `void LeftMove(int* current)`
**Purpose**: Moves cursor one position left in text input.

**Parameters**:
- `current` - Pointer to current cursor position

**Logic**: Decrements position if greater than 0

---

### `void RightMove(int* current, int* last)`
**Purpose**: Moves cursor one position right in text input.

**Parameters**:
- `current` - Pointer to current cursor position
- `last` - Pointer to last character position

**Logic**: Increments position if less than last position

---

### `void PressedHome(int* current, int* last)`
**Purpose**: Moves cursor to beginning of line.

**Parameters**:
- `current` - Pointer to current cursor position
- `last` - Pointer to last character position

**Logic**: Sets current to 0

---

### `void PressedEnd(int* current, int* last)`
**Purpose**: Moves cursor to end of line.

**Parameters**:
- `current` - Pointer to current cursor position
- `last` - Pointer to last character position

**Logic**: Sets current to last position

---

### `void DeleteChar(char* line, int* current, int* last)`
**Purpose**: Deletes character at cursor position (Delete key).

**Parameters**:
- `line` - Character buffer
- `current` - Pointer to cursor position
- `last` - Pointer to last character index

**Logic**: Shifts characters left from cursor position

---

### `void Backspace(char* line, int* current, int* last)`
**Purpose**: Deletes character before cursor (Backspace key).

**Parameters**:
- `line` - Character buffer
- `current` - Pointer to cursor position
- `last` - Pointer to last character index

**Logic**: Moves cursor back and shifts characters left

---

### `string trim(const char* s)`
**Purpose**: Removes leading and trailing spaces from string.

**Parameters**:
- `s` - C-string to trim

**Returns**: Trimmed string

**Logic**:
1. Finds first non-space character
2. Removes trailing spaces
3. Returns cleaned string

---

## UserManager Class Methods

### `bool login(sqlite3* db)`
**Purpose**: Interactive login form with email/password authentication.

**Parameters**:
- `db` - SQLite database pointer

**Returns**: `true` if login successful, `false` if user cancels

**UI Features**:
- Focused field highlighting (blue background)
- Password masking with asterisks
- Navigation with arrow keys or Tab
- Options: Email field, Password field, Sign Up button, Back button

**Input Handling**:
- **Arrow Up/Down**: Navigate between fields/buttons
- **Tab**: Cycle through options
- **Enter**: 
  - On fields: Open multi-line editor for input
  - On "Sign Up": Call `signup()`
  - On "Back": Return to main menu
- **ESC**: Return to main menu

**Validation**:
- Checks if email and password are not empty
- Queries database: `SELECT id FROM users WHERE email=? AND password=?`

**On Success**:
- Sets `currentUserId` from database
- Sets `currentUserEmail` to entered email
- Sets `isLoggedIn = true`
- Displays success message

**On Failure**:
- Shows error message
- Clears fields
- Allows retry

**Connected to**:
- Uses `multiLineEditor()` from `ConsoleUtils.h` for input
- Updates global variables in `main.cpp`
- Called by `executeMenuAction()` in `main.cpp`

---

### `bool signup(sqlite3* db)`
**Purpose**: User registration with email/password validation.

**Parameters**:
- `db` - SQLite database pointer

**Returns**: `true` if signup successful, `false` if user cancels

**UI Features**:
- Similar layout to login form
- Email and password fields
- Back button
- Field highlighting

**Input Handling**:
- **Arrow Up/Down**: Navigate fields
- **Tab**: Cycle through options
- **Enter**: Edit fields and submit
- **ESC**: Cancel and return

**Validation Rules**:

1. **Email Format** (regex):
   ```regex
   ^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$
   ```

2. **Password Strength** (regex):
   ```regex
   ^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[^A-Za-z0-9]).{8,}$
   ```
   - Minimum 8 characters
   - At least one lowercase letter
   - At least one uppercase letter
   - At least one digit
   - At least one special character

3. **Email Uniqueness**:
   - Checks database: `SELECT id FROM users WHERE email=?`
   - Rejects if email already exists

**Database Operation**:
```sql
INSERT INTO users (email, password, isAdmin) VALUES (?, ?, 0);
```
- New users are not admins by default (`isAdmin = 0`)

**Connected to**:
- Uses `multiLineEditor()` from `ConsoleUtils.h`
- Called by `login()` when "Sign Up" is selected
- Called by `executeMenuAction()` in `main.cpp`
- Inserts into `users` table managed by `DBManager`

## Data Flow

### Login Flow
```
User selects Login
    │
    ├─ Display login form
    │
    ├─ User enters email/password
    │
    ├─ Query database for credentials
    │
    ├─ If found:
    │   ├─ Set global variables (isLoggedIn, currentUserId, currentUserEmail)
    │   └─ Return to main menu with user privileges
    │
    └─ If not found:
        ├─ Show error message
        └─ Allow retry
```

### Signup Flow
```
User selects Sign Up
    │
    ├─ Display signup form
    │
    ├─ User enters email/password
    │
    ├─ Validate email format
    │
    ├─ Validate password strength
    │
    ├─ Check email uniqueness
    │
    ├─ If all valid:
    │   ├─ Insert into database
    │   ├─ Show success message
    │   └─ Return to login
    │
    └─ If invalid:
        ├─ Show specific error
        └─ Allow retry
```

## File Connections

| Connected File | Purpose |
|----------------|---------|
| `UserManager.h` | Class declaration |
| `main.cpp` | Calls login/signup, uses global session variables |
| `ConsoleUtils.h` | `multiLineEditor()` for input, `gotoxy()`, `textattr()` |
| `PropertyManager.h` | Uses login status to control contact info visibility |
| `DBManager.h` | Database contains `users` table |
| `sqlite3.h` | Direct SQL queries for authentication |

## Security Considerations

⚠️ **Current Implementation Issues**:
1. **Plain Text Passwords**: Passwords stored without hashing
2. **SQL Injection**: Uses prepared statements (✓ Good)
3. **No Rate Limiting**: Unlimited login attempts allowed
4. **No Session Management**: Basic boolean flag only

**Recommendations for Production**:
- Implement password hashing (bcrypt, argon2)
- Add rate limiting for failed attempts
- Use session tokens instead of global variables
- Add 2FA support
- Log authentication attempts
