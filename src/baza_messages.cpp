#include <iostream>
#include <sqlite3.h>
#include <string>
#include <iostream>
using namespace std;

int main()
{
    // Do not remove the database here. Removing the DB will delete any
    // tables created by other initialisation code (e.g. USERS).
    // If you really need to recreate the DB, run a dedicated init script
    // that creates all required tables. For safety we skip unlinking here.
    sqlite3 *DB;
    string sqlmessages = "CREATE TABLE IF NOT EXISTS MESSAGES("
                         "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "SENDER TEXT NOT NULL, "
                         "RECEIVER TEXT NOT NULL, "
                         "CONTENT TEXT NOT NULL, "
                         "TIMESTAMP TEXT DEFAULT CURRENT_TIMESTAMP);";
    int exit = 0;
    exit = sqlite3_open("chat_database.db", &DB);
    char *messaggeError;
    exit = sqlite3_exec(DB, sqlmessages.c_str(), NULL, 0, &messaggeError);
    if (exit != SQLITE_OK)
    {
        std::cerr << "Error Create Table MESSAGES: " << messaggeError << std::endl;
        sqlite3_free(messaggeError);
    }
    else
    {
        std::cout << "Table created successfully!" << std::endl;
    }
    sqlite3_close(DB);
    return 0;
}
