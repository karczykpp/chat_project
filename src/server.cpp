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
using json = nlohmann::json;
using namespace std;

int main(void)
{
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  socklen_t addr_size;

  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (serverSocket == 1)
  {
    perror("Socket creation failed");
    return 1;
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1100);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
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

  while (true)
  {
    addr_size = sizeof clientAddr;
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addr_size);
    if (clientSocket == -1)
    {
      perror("Accept failed");
      continue;
    }
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    int n = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
      cout << "Received data: " << buffer << std::endl;
      try
      {
        json received_json = json::parse(buffer);
        string command = received_json.value("command", "UNKNOWN");
        json response;

        if (command == "REGISTER")
        {
          string user = received_json.value("username", "");
          string password = received_json.value("password", "");

          sqlite3* DB;
          char *messaggeError;
          int exit = sqlite3_open("chat_database.db", &DB);
          string sqlInsert = "INSERT INTO USERS (USERNAME, PASSWORD) VALUES ('" + user + "', '" + password + "');";
          exit = sqlite3_exec(DB, sqlInsert.c_str(), NULL, 0, &messaggeError);
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
        }
        else
        {
          response["status"] = "ERROR";
          response["message"] = "Unknown command.";
        }

        string response_str = response.dump();
        send(clientSocket, response_str.c_str(), response_str.size(), 0);
      }
      catch (json::parse_error &e)
      {
        cerr << "JSON parse error: " << e.what() << std::endl;
      }
    }

    close(clientSocket);
    cout << "Connection closed." << std::endl;
  }
  close(serverSocket);
  return 0;
}