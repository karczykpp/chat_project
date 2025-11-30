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

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
using json = nlohmann::json;
using namespace std;

int serverSocket;
struct sockaddr_in serverAddr, clientAddr;
socklen_t addr_size;

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

json loginStage(json received_json)
{
  json response;
  string user = received_json.value("username", "");
  string password = received_json.value("password", "");

  sqlite3 *DB;
  char *messaggeError;
  int exit = sqlite3_open("chat_database.db", &DB);
  string query = "SELECT USERNAME, PASSWORD FROM USERS WHERE USERNAME='" + user + "';";
  sqlite3_stmt *stmt;
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
        response["status"] = "SUCCESS";
        response["message"] = "Login successful.";
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

void *socketThread(void *arg)
{
  char buffer[2048];
  int newSocket = *((int *)arg);
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
          response = loginStage(received_json);
        }
        else
        {
          response["status"] = "ERROR";
          response["message"] = "Unknown command.";
        }

        string response_str = response.dump();
        send(newSocket, response_str.c_str(), response_str.size(), 0);
      }
      catch (json::parse_error &e)
      {
        cerr << "JSON parse error: " << e.what() << std::endl;
      }
    }
    else
    {
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

  if (listen(serverSocket, 10) == 0)
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
    if (pthread_create(&thread_id, NULL, socketThread, &clientSocket) != 0)
      printf("Failed to create thread\n");

    pthread_detach(thread_id);
  }
  close(serverSocket);
  return 0;
}