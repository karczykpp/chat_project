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
#include <ctime>

using json = nlohmann::json;
using namespace std;

class UserManager
{
private:
  vector<pair<string, int>> loggedInUsers;
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

public:
  void addUser(int socketfd, string username)
  {
    pthread_mutex_lock(&lock);
    loggedInUsers.push_back(make_pair(username, socketfd));
    cout << "[UserManager] Dodano: " << username << " z socketfd: " << socketfd << endl;
    pthread_mutex_unlock(&lock);
  }

  void removeUser(int socketfd)
  {
    pthread_mutex_lock(&lock);
    for (auto it = loggedInUsers.begin(); it != loggedInUsers.end(); ++it)
    {
      if (it->second == socketfd)
      {
        cout << "[UserManager] Usunięto: " << it->first << " z socketfd: " << socketfd << endl;
        loggedInUsers.erase(it);
        break;
      }
    }
    pthread_mutex_unlock(&lock);
  }

  int getSocket(string username)
  {
    pthread_mutex_lock(&lock);
    int foundSocket = -1;

    for (const auto &user : loggedInUsers)
    {
      if (user.first == username)
      {
        foundSocket = user.second;
        break;
      }
    }
    pthread_mutex_unlock(&lock);
    return foundSocket;
  }

  string getUsername(int socketfd)
  {
    pthread_mutex_lock(&lock);
    string foundName = "";

    for (const auto &user : loggedInUsers)
    {
      if (user.second == socketfd)
      {
        foundName = user.first;
        break;
      }
    }
    pthread_mutex_unlock(&lock);
    return foundName;
  }

  string getOnlineList()
  {
    pthread_mutex_lock(&lock);
    string list = "";
    for (const auto &user : loggedInUsers)
    {
      list += user.first + ",";
    }
    if (!list.empty())
    {
      list.pop_back();
    }
    pthread_mutex_unlock(&lock);
    return list;
  }

  void broadcast(string message, int senderSocket)
  {
    pthread_mutex_lock(&lock);
    for (const auto &user : loggedInUsers)
    {
      if (user.second != senderSocket)
      {
        string messageToSend = message + "\n";
        send(user.second, messageToSend.c_str(), messageToSend.size(), 0);
      }
    }
    pthread_mutex_unlock(&lock);
  }

  void broadcastToUser(string message, int senderSocket, int receiverSocket)
  {
    pthread_mutex_lock(&lock);
    string messageToSend = message + "\n";
    if (receiverSocket != -1 && receiverSocket != senderSocket) {
      send(receiverSocket, messageToSend.c_str(), messageToSend.size(), 0);
    }
    pthread_mutex_unlock(&lock);
  }
};

int serverSocket;
struct sockaddr_in serverAddr, clientAddr;
socklen_t addr_size;

UserManager userManager;

json registerStage(json received_json)
{
  json response;
  string user = received_json.value("username", "");
  string password = received_json.value("password", "");

  sqlite3 *DB;
  char *messaggeError;
  int exit = sqlite3_open("chat_database.db", &DB);
  string sqlInsert = "INSERT INTO USERS (USERNAME, PASSWORD) VALUES ('" + user + "', '" + password + "');";
  exit = sqlite3_exec(DB, sqlInsert.c_str(), NULL, 0, &messaggeError);
  cout << exit << endl;
  if (exit != SQLITE_OK)
  {
    response["status"] = "ERROR";
    response["message"] = "Error registering user: " + string(messaggeError);
    sqlite3_free(messaggeError);
  }
  else
  {
    response["status"] = "SUCCESS";
    response["message"] = "User registered successfully.";
  }
  sqlite3_close(DB);
  return response;
}

json loginStage(json received_json, int socketfd)
{
  json response;
  string user = received_json.value("username", "");
  string password = received_json.value("password", "");

  sqlite3 *DB;
  char *messaggeError;
  int exit = sqlite3_open("chat_database.db", &DB);
  string query = "SELECT USERNAME, PASSWORD FROM USERS WHERE USERNAME='" + user + "';";
  sqlite3_stmt *stmt;
  string usersList = "";
  if (sqlite3_prepare_v2(DB, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
  {
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      const unsigned char *usernameVal = sqlite3_column_text(stmt, 0);
      const unsigned char *passwordVal = sqlite3_column_text(stmt, 1);
      string userDB = string(reinterpret_cast<const char *>(usernameVal));
      string passDB = string(reinterpret_cast<const char *>(passwordVal));
      if (passDB == password)
      {
        string queryUsers = "SELECT USERNAME FROM USERS;";
        sqlite3_stmt *stmtUsers;
        if (sqlite3_prepare_v2(DB, queryUsers.c_str(), -1, &stmtUsers, NULL) == SQLITE_OK)
        {
          while (sqlite3_step(stmtUsers) == SQLITE_ROW)
          {
            const unsigned char *usernameVal = sqlite3_column_text(stmtUsers, 0);
            string userIter = string(reinterpret_cast<const char *>(usernameVal));
            if (userIter != user)
              usersList += userIter + ",";
          }
          if (!usersList.empty())
          {
            usersList.pop_back();
          }
          response["all_users"] = usersList;
        }
        sqlite3_finalize(stmtUsers);
        string groupsList = "";
        string queryGroups = "SELECT g.NAME FROM GROUPS g "
                             "JOIN GROUP_MEMBERS gm ON g.ID = gm.GROUP_ID "
                             "WHERE gm.USERNAME = '" + user + "';";
        
        sqlite3_stmt *stmtGroups;
        if (sqlite3_prepare_v2(DB, queryGroups.c_str(), -1, &stmtGroups, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmtGroups) == SQLITE_ROW) {
                const unsigned char *groupNameVal = sqlite3_column_text(stmtGroups, 0);
                string groupName = string(reinterpret_cast<const char *>(groupNameVal));
                groupsList += groupName + " (Grupa),";
            }
        }
        sqlite3_finalize(stmtGroups);

        string finalDisplayList = usersList;
        if (!groupsList.empty()) {
            if (!finalDisplayList.empty()) finalDisplayList += ",";
            finalDisplayList += groupsList;
        }
        
        if (!finalDisplayList.empty() && finalDisplayList.back() == ',') {
            finalDisplayList.pop_back();
        }

        response["all_users"] = finalDisplayList;
        userManager.addUser(socketfd, user);
        response["status"] = "SUCCESS";
        response["message"] = "Login successful.";
        response["online_users"] = userManager.getOnlineList();
        json broadcast_msg;
        broadcast_msg["command"] = "USER_ONLINE";
        broadcast_msg["username"] = user;
        broadcast_msg["online_users"] = userManager.getOnlineList();
        string response_str = broadcast_msg.dump();
        userManager.broadcast(response_str, socketfd);
      }
      else
      {
        response["status"] = "ERROR";
        response["message"] = "Incorrect password.";
      }
    }
    else
    {
      response["status"] = "USER_NOT_FOUND";
      response["message"] = "User not found.";
    }
  }
  else
  {
    cerr << "Błąd w zapytaniu SQL" << endl;
  }
  sqlite3_finalize(stmt);
  return response;
}

json logoutStage(int socketfd)
{
  json response;
  string username = userManager.getUsername(socketfd);
  cout << "[LogoutStage] User to logout: " << username << " with socketfd: " << socketfd << endl;
  cout << userManager.getOnlineList() << endl;
  if (!username.empty())
  {
    userManager.removeUser(socketfd);
    response["status"] = "SUCCESS";
    response["message"] = "Logout successful.";
    json broadcast_msg;
    broadcast_msg["command"] = "USER_ONLINE";
    broadcast_msg["username"] = username;
    broadcast_msg["online_users"] = userManager.getOnlineList();
    cout << broadcast_msg.dump() << endl;
    string response_str = broadcast_msg.dump() + "\n";
    userManager.broadcast(response_str, socketfd);
  }
  else
  {
    response["status"] = "ERROR";
    response["message"] = "User not logged in.";
  }
  return response;
}

json getMessages(json received_json) {
    string receiver = received_json.value("receiver", "");
    string sender = received_json.value("sender", "");
    
    string cleanReceiver = receiver;
    size_t pos = cleanReceiver.find(" (Grupa)");
    if (pos != string::npos) cleanReceiver.erase(pos);

    sqlite3 *DB;
    sqlite3_open("chat_database.db", &DB);

    int groupID = -1;
    sqlite3_stmt *stG;
    sqlite3_prepare_v2(DB, "SELECT ID FROM GROUPS WHERE NAME = ?;", -1, &stG, nullptr);
    sqlite3_bind_text(stG, 1, cleanReceiver.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stG) == SQLITE_ROW) groupID = sqlite3_column_int(stG, 0);
    sqlite3_finalize(stG);

    sqlite3_stmt *stmt;
    if (groupID != -1) {
        // Pobieramy historię grupy po ID (teraz pola GROUP_ID nie będą już null)
        string sql = "SELECT SENDER, CONTENT, TIMESTAMP FROM MESSAGES WHERE GROUP_ID = ? ORDER BY TIMESTAMP ASC;";
        sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, groupID);
    } else {
        string sql = "SELECT SENDER, CONTENT, TIMESTAMP FROM MESSAGES WHERE (SENDER=? AND RECEIVER=?) OR (SENDER=? AND RECEIVER=?) ORDER BY TIMESTAMP ASC;";
        sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, receiver.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, sender.c_str(), -1, SQLITE_STATIC);
    }

    json history = json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json m;
        m["sender"] = (const char*)sqlite3_column_text(stmt, 0);
        m["content"] = (const char*)sqlite3_column_text(stmt, 1);
        m["time"] = (const char*)sqlite3_column_text(stmt, 2);
        history.push_back(m);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    if (history.empty()) {
        json emptyMsg;
        emptyMsg["status"] = "EMPTY";
        history.push_back(emptyMsg);
    }
    return history;
}

json sendMessage(json received_json) {
    json response;
    string receiver = received_json.value("receiver", "");
    string sender = received_json.value("sender", "");
    string content = received_json.value("content", "");

    string cleanReceiver = receiver;
    size_t pos = cleanReceiver.find(" (Grupa)");
    if (pos != string::npos) cleanReceiver.erase(pos);

    sqlite3 *DB;
    sqlite3_open("chat_database.db", &DB);

    int groupID = -1;
    sqlite3_stmt *stG;
    sqlite3_prepare_v2(DB, "SELECT ID FROM GROUPS WHERE NAME = ?;", -1, &stG, nullptr);
    sqlite3_bind_text(stG, 1, cleanReceiver.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stG) == SQLITE_ROW) groupID = sqlite3_column_int(stG, 0);
    sqlite3_finalize(stG);

    string sql;
    sqlite3_stmt *stmt;
    if (groupID != -1) {
        sql = "INSERT INTO MESSAGES (SENDER, GROUP_ID, CONTENT) VALUES (?, ?, ?);";
        sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, groupID);
        sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_STATIC);
    } else {
        sql = "INSERT INTO MESSAGES (SENDER, RECEIVER, CONTENT) VALUES (?, ?, ?);";
        sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_STATIC);
    }

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        response["status"] = "SUCCESS";
        if (groupID != -1) response["group_id"] = groupID;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return response;
}
json createGroup(json received_json)
{
  json response;
  string groupName = received_json.value("group_name", "");
  string creator = received_json.value("created_by", "");
  auto members = received_json.value("members", vector<string>());
  sqlite3 *DB;
  char *messaggeError;
  int exit = sqlite3_open("chat_database.db", &DB);

  if (exit != SQLITE_OK) {
      return {{"status", "ERROR"}, {"message", "Nie można otworzyć bazy danych"}};
  }
  string sqlGroup = "INSERT INTO GROUPS (NAME, CREATED_BY) VALUES ('" + groupName + "', '" + creator + "');";
  exit = sqlite3_exec(DB, sqlGroup.c_str(), NULL, 0, &messaggeError);
  if (exit != SQLITE_OK)
  {
      response["status"] = "ERROR";
      response["message"] = "Błąd przy tworzeniu grupy: " + string(messaggeError);
      sqlite3_free(messaggeError);
      sqlite3_close(DB);
      return response;
  }
  sqlite3_int64 groupID = sqlite3_last_insert_rowid(DB);
  bool allAdded = true;
  for (const string& member : members)
  {
      string sqlMember = "INSERT INTO GROUP_MEMBERS (GROUP_ID, USERNAME) VALUES (" + to_string(groupID) + ", '" + member + "');";
      exit = sqlite3_exec(DB, sqlMember.c_str(), NULL, 0, &messaggeError);
      if (exit != SQLITE_OK) {
          allAdded = false;
          sqlite3_free(messaggeError);
      }
  }

  if (allAdded)
  {
      response["status"] = "SUCCESS";
      response["message"] = "Grupa '" + groupName + "' została utworzona pomyślnie.";
  }
  else
  {
      response["status"] = "PARTIAL_SUCCESS";
      response["message"] = "Grupa utworzona, ale wystąpił problem z dodaniem niektórych członków.";
  }

  sqlite3_close(DB);
  return response;
}

void *socketThread(void *arg)
{
  char buffer[2048];
  int newSocket = *((int *)arg);
  cout << "New connection accepted, socket fd: " << newSocket << std::endl;
  while (true)
  {
    if (newSocket == -1)
    {
      perror("Accept failed");
      continue;
    }
    memset(buffer, 0, sizeof(buffer));
    int n = recv(newSocket, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
      cout << "Received data: " << buffer << std::endl;
      try
      {
        json received_json = json::parse(buffer);
        string command = received_json["command"];
        json response;

        if (command == "REGISTER")
        {
          response = registerStage(received_json);
        }
        else if (command == "LOGIN")
        {
          response = loginStage(received_json, newSocket);
        }
        else if (command == "LOGOUT")
        {
          response = logoutStage(newSocket);
          cout << "Connection closed." << std::endl;
        }
        else if (command == "GET_MESSAGES")
        {
          response = getMessages(received_json);
          //cout << "Historia: " << response << endl;
        }
        else if (command == "SEND_MESSAGE")
        {
          response = sendMessage(received_json);
          if (response["status"] == "SUCCESS") {
              json history = getMessages(received_json);
              string historyStr = history.dump() + "\n";
              
              if (response.contains("group_id")) {
                  int gID = response["group_id"];
                  sqlite3 *db;
                  sqlite3_open("chat_database.db", &db);
                  sqlite3_stmt *stM;
                  string sqlM = "SELECT USERNAME FROM GROUP_MEMBERS WHERE GROUP_ID = " + to_string(gID) + ";";
                  if (sqlite3_prepare_v2(db, sqlM.c_str(), -1, &stM, NULL) == SQLITE_OK) {
                      while (sqlite3_step(stM) == SQLITE_ROW) {
                          string mName = (const char*)sqlite3_column_text(stM, 0);
                          int mSock = userManager.getSocket(mName);
                          if (mSock != -1) {
                              userManager.broadcastToUser(historyStr, -1, mSock);
                          }
                      }
                  }
                  sqlite3_finalize(stM);
                  sqlite3_close(db);
              } else {
                  string receiver = received_json.value("receiver", "");
                  int receiverSocket = userManager.getSocket(receiver);
                  
                  if (receiverSocket != -1) {
                      userManager.broadcastToUser(historyStr, newSocket, receiverSocket);
                  }
                  userManager.broadcastToUser(historyStr, -1, newSocket);
              }
          }
        }
        else if (command == "CREATE_GROUP")
        {
          response = createGroup(received_json);
        }
        else
        {
          response["status"] = "ERROR";
          response["message"] = "Unknown command.";
        }

        string response_str = response.dump() + "\n";
        //cout << "Odpowiedz" << response_str << endl;
        send(newSocket, response_str.c_str(), response_str.size(), 0);
      }
      catch (json::parse_error &e)
      {
        cerr << "JSON parse error: " << e.what() << std::endl;
      }
    }
    else
    {
      userManager.removeUser(newSocket);
      close(newSocket);
      cout << "Connection closed." << std::endl;
      break;
    }
  }
  printf("Exit socketThread \n");

  pthread_exit(NULL);
}

int main(void)
{
  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1)
  {
    perror("Socket creation failed");
    return 1;
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1104);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  int opt = 1;
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (::bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
  {
    perror("Bind failed");
    return 1;
  }

  if (listen(serverSocket, 100) == 0)
  {
    cout << "Listening on port 1100..." << std::endl;
  }
  else
  {
    cout << "Listen failed!" << std::endl;
  }
  pthread_t thread_id;

  while (true)
  {
    addr_size = sizeof clientAddr;
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addr_size);
    cout << clientSocket << endl;
    if (pthread_create(&thread_id, NULL, socketThread, &clientSocket) != 0)
      printf("Failed to create thread\n");

    pthread_detach(thread_id);
  }
  close(serverSocket);
  return 0;
}