#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../json.hpp"
#include <iostream>
#include <sqlite3.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <mutex>
#include <utility>
#include <sstream>
#include <iomanip>
#include <ctime>
using json = nlohmann::json;
using namespace std;

int main()
{
    string sender = "wuja";   // JA
    string receiver = "kuba"; // ON

    sqlite3 *DB;
    sqlite3_stmt *stmt;

    if (sqlite3_open("chat_database.db", &DB) != SQLITE_OK)
    {
        cerr << "Cannot open database\n";
        return 1;
    }
    string sql =
        "SELECT ID, SENDER, RECEIVER, CONTENT, TIMESTAMP FROM MESSAGES "
        "WHERE (SENDER=? AND RECEIVER=?) "
        "OR (SENDER=? AND RECEIVER=?) "
        "ORDER BY TIMESTAMP ASC;";

    int rc = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error: " << sqlite3_errmsg(DB) << endl;
        return 1;
    }

    sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, receiver.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, sender.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        string timeStr = (const char *)sqlite3_column_text(stmt, 4);
        string dbSender = (const char *)sqlite3_column_text(stmt, 1);
        string content = (const char *)sqlite3_column_text(stmt, 3);

        // ====== PARSOWANIE DATY ======
        std::tm tm{};
        std::istringstream ss(timeStr);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

        int rok = tm.tm_year + 1900;
        int miesiac = tm.tm_mon + 1;
        int dzien = tm.tm_mday;
        int godzina = tm.tm_hour;
        int minuta = tm.tm_min;
        int sekunda = tm.tm_sec;
        // =============================

        // DEBUG – zobaczysz, że to działa
        cout << "DATA: "
             << dzien << "." << miesiac << "." << rok << " "
             << godzina << ":" << minuta << ":" << sekunda << endl;

        // normalne wypisanie wiadomości
        if (dbSender == sender)
            cout << "TY: " << content << endl;
        else
            cout << dbSender << ": " << content << endl;

        cout << "--------------------" << endl;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(DB);

    return 0;
}