#include <sqlite3.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
using namespace std;

int main()
{
    sqlite3 *DB;
    int exit = sqlite3_open("chat_database.db", &DB);
    string query = "SELECT USERNAME, PASSWORD FROM USERS WHERE USERNAME='Jakub Karcz';";
    sqlite3_stmt *stmt; 
    if (sqlite3_prepare_v2(DB, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
    {
        int ilosc = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char *usernameVal = sqlite3_column_text(stmt, 0);
            const unsigned char *passwordVal = sqlite3_column_text(stmt, 1);
            string user = string(reinterpret_cast<const char *>(usernameVal));
            string pass = string(reinterpret_cast<const char *>(passwordVal));
            cout << "Znalazłem usera: " << user << " z hasłem: " << pass << endl;
            ilosc++;
        }
        if (ilosc == 0)
            cout<<"Brak dalszych wyników."<<endl;
    }
    else
    {
        cerr << "Błąd w zapytaniu SQL" << endl;
    }
    sqlite3_finalize(stmt);
    return 0;
}