#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "../json.hpp"
using namespace std;
using json = nlohmann::json;


int main(int argc, char *argv[])
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
  {
    perror("Socket creation failed");
    return 1;
  }
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1101);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
  {
    perror("Connection to server failed");
    close(sock);
    return 1;
  }

  string user, password;
  cout << "Enter username: ";
  getline(cin, user);
  cout << "Enter password: ";
  getline(cin, password);
  json request;
  request["command"] = "REGISTER";
  request["username"] = user;
  request["password"] = password;

  string message = request.dump();
  send(sock, message.c_str(), message.size(), 0);

  char buffer[2048];
  memset(buffer, 0, sizeof(buffer));
  int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
  if (n > 0)
  {
    json response = json::parse(buffer);
    cout << "Wynik z serwera: " << response["status"] << std::endl;
    cout << "Message from server: " << response["message"] << std::endl;
  }
  close(sock);
  return 0;
}