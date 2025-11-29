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
  json request;
  string user, password;
  int wybor;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
  {
    perror("Socket creation failed");
    return 1;
  }
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1104);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
  {
    perror("Connection to server failed");
    close(sock);
    return 1;
  }
  cout << "Wybierz opcję:\n 1. Rejestracja\n 2. Logowanie\n"
       << std::endl;
  cin >> wybor;
  cin.ignore();
  switch (wybor)
  {
  case 1:
    cout << "Wybrano rejestrację."<<endl;
    cout << "Podaj dane do rejestracji.\n Podaj użytkownika: " <<endl;
    getline(cin, user);
    cout << "Podaj hasło: " <<endl;
    getline(cin, password);
    request["command"] = "REGISTER";
    request["username"] = user;
    request["password"] = password;
    break;
  case 2:
    cout << "Wybrano logowanie."<<endl;
    cout << "Podaj dane do logowania. \nPodaj użytkownika: " <<endl;
    getline(cin, user);
    cout << "Podaj hasło: " <<endl;
    getline(cin, password);
    request["command"] = "LOGIN";
    request["username"] = user;
    request["password"] = password;
    break;

  default:
    break;
  }

  string message = request.dump();
  send(sock, message.c_str(), message.size(), 0);

  char buffer[2048];
  memset(buffer, 0, sizeof(buffer));
  int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
  if (n > 0)
  {
    json response = json::parse(buffer);
    cout << "Wynik z serwera: " << response["status"] << endl;
    cout << "Message from server: " << response["message"] << endl;
    if (response["status"] == "USER_NOT_FOUND")
    {
      cout << "Nie znaleziono użytkownika. Proszę się zarejestrować." <<endl;
      cout << "Podaj dane do rejestracji.\n Podaj użytkownika: " <<endl;
      getline(cin, user);
      cout << "Podaj hasło: " <<endl;
      getline(cin, password);
      request["command"] = "REGISTER";
      request["username"] = user;
      request["password"] = password;
      message = request.dump();
      send(sock, message.c_str(), message.size(), 0);
      memset(buffer, 0, sizeof(buffer));
      n = recv(sock, buffer, sizeof(buffer) - 1, 0);
      if (n > 0)
      {
        json reg_response = json::parse(buffer);
        cout << "Wynik z serwera: " << reg_response["status"] << endl;
        cout << "Message from server: " << reg_response["message"] << endl; 
      }
    }
    if (response["status"] == "SUCCESS")
    {
      cout << "Zalogowano pomyślnie!" << endl;
    }
  }
  close(sock);
  return 0;
}