#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in sa;
    int socketfd;
    int port =  atoi(argv[2]);
    socketfd = socket(PF_INET, SOCK_DGRAM, 0);
    if(socketfd == -1){
        perror("cant create");
    } 
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr =inet_addr(argv[1]);
    sa.sin_port = htons(port);
    char buff[256];
    int len = sizeof sa;
    if (connect(socketfd, (struct sockaddr *)&sa, sizeof sa) == -1){
        printf("cannot connect");
    }
    while(1){
        bzero(buff,256);
        fgets(buff,256,stdin);
        if(sendto(socketfd, buff, sizeof buff, 0, (struct sockaddr *)&sa, len) == -1){
            printf("cannot send");
        }
        printf("wiadomosc od klienta: %s\n", buff);
        bzero(buff,256);
        if(recvfrom(socketfd, buff, 256, MSG_WAITALL, (struct sockaddr *)&sa, &len) == -1){
            printf("wiadomosc od servera: %s\n", buff);
        }
        printf("wiadomosc od servera: %s\n", buff);
    }
}