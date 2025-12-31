    #pragma once
    #include "sqlite3.h"

    class UserManager {
    public:
        bool login(sqlite3* db);
         bool signup(sqlite3* db);
    };
