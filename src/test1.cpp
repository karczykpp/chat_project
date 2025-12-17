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

    json chat_history = json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        json msg;
        const char* sender_val    = (const char*)sqlite3_column_text(stmt, 1);
        const char* receiver_val  = (const char*)sqlite3_column_text(stmt, 2);
        const char* content_val   = (const char*)sqlite3_column_text(stmt, 3);
        const char* timestamp_val = (const char*)sqlite3_column_text(stmt, 4);

        msg["sender"]    = sender_val    ? sender_val    : "";
        msg["receiver"]  = receiver_val  ? receiver_val  : "";
        msg["content"]   = content_val   ? content_val   : "";
        msg["timestamp"] = timestamp_val ? timestamp_val : "";
        chat_history.push_back(msg);
    }
    string json_payload = chat_history.dump(4);
    cout<<json_payload<<endl;
    sqlite3_finalize(stmt);
    sqlite3_close(DB);

    return 0;
}