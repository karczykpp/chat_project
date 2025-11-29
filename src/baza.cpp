#include <iostream>
#include <sqlite3.h>
#include <string>
#include <iostream>
using namespace std;

int main()
{
    if (remove("chat_database.db") != 0) {
        cout << "Nie usunieto pliku (moze nie istnial), tworze nowy..." << endl;
    } else {
        cout << "Poprzednia baza usunieta pomyslnie." << endl;
    }
    sqlite3* DB;
    string sqlUser = "CREATE TABLE IF NOT EXISTS USERS("
                     "USERNAME TEXT PRIMARY KEY, "
                     "PASSWORD TEXT NOT NULL);";
    int exit = 0;
    exit = sqlite3_open("chat_database.db", &DB);
    char* messaggeError;
    exit = sqlite3_exec(DB, sqlUser.c_str(), NULL, 0, &messaggeError);
    if (exit != SQLITE_OK) 
    {
        std::cerr << "Error Create Table USERS" << std::endl;
        sqlite3_free(messaggeError);
    }
    else
    {
        std::cout << "Table created successfully!" << std::endl;
    }
    sqlite3_close(DB);
    return 0;
}
