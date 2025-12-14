#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<pthread.h>


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void * thread(void *arg){
    printf("new socket\n");

    int newsocket = *((int *)arg);
    int n;
    char buff[256];
    while(1){
        bzero(buff,256);
        n = read(newsocket, buff, sizeof buff);
        printf("%i\n", n);
        if (n<1)
            break;
        printf("wiadomosc od klienta: %s\n", buff);
        write(newsocket, buff, sizeof buff);
    }
    bzero(buff,256);
}

int main(void) {

    struct sockaddr_in sa, client;
    memset(&sa, 0, sizeof sa);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);

    int socketfd = socket(PF_INET, SOCK_DGRAM,0);

    char buff[256];
    int n;
    int len = sizeof client;

    if(bind(socketfd, (struct sockaddr *)&sa, sizeof sa) == -1){
        printf("cannot bind");
    }
    
    while(1){
        bzero(buff,256);
        if(recvfrom(socketfd, (char*)buff,256,MSG_WAITALL, (struct sockaddr *)&client, &len) == -1){
            printf("cannor receive");
        }
        printf("wiadomosc od klienta %s\n", buff);
        bzero(buff,256);
        fgets(buff, 256, stdin);
        if(sendto(socketfd, buff, sizeof buff, 0, (struct sockaddr *)&client, len) == -1){
            printf("cannot send");
        }

    }
}