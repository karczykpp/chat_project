#include <iostream>
#include <sqlite3.h>
#include <string>
#include <cstdio>
using namespace std;

int main()
{
    if (remove("chat_database.db") != 0) {
        cout << "Info: Nie usunieto pliku (moze nie istnial lub jest otwarty w innym programie)." << endl;
    } else {
        cout << "Info: Poprzednia baza usunieta pomyslnie." << endl;
    }

    sqlite3* DB;
    int exit = 0;
    char* messaggeError;

    exit = sqlite3_open("chat_database.db", &DB);
    if (exit != SQLITE_OK) {
        cerr << "Blad otwarcia bazy!" << endl;
        return -1;
    }

    string sqlUser = "CREATE TABLE IF NOT EXISTS USERS("
                     "USERNAME TEXT PRIMARY KEY, "
                     "PASSWORD TEXT NOT NULL);";

    exit = sqlite3_exec(DB, sqlUser.c_str(), NULL, 0, &messaggeError);

    if (exit != SQLITE_OK) 
    {
        std::cerr << "Blad przy tworzeniu tabeli USERS: " << messaggeError << std::endl;
        sqlite3_free(messaggeError);
    }
    else
    {
        std::cout << "Tabela USERS utworzona!" << std::endl;
    }

    string sqlmessages = "CREATE TABLE IF NOT EXISTS MESSAGES("
                         "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "SENDER TEXT NOT NULL, "
                         "RECEIVER TEXT NOT NULL, "
                         "CONTENT TEXT NOT NULL, "
                         "TIMESTAMP TEXT DEFAULT CURRENT_TIMESTAMP);";

    exit = sqlite3_exec(DB, sqlmessages.c_str(), NULL, 0, &messaggeError);

    if (exit != SQLITE_OK)
    {
        std::cerr << "Blad przy tworzeniu tabeli MESSAGES: " << messaggeError << std::endl;
        sqlite3_free(messaggeError);
    }
    else
    {
        std::cout << "Tabela MESSAGES utworzona!" << std::endl;
    }

    sqlite3_close(DB);
    return 0;
}
