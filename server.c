#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "http.h"

#define PORT 1234
#define BACKLOG 10

#ifndef BUFF
    #define BUFF_SIZE 256
    #define BUFF
#endif

void cleanExit();
void open();
void part1(int);
void part2(int);

/**
 * TODO
 * 1. Content-Type Matcher.
 * 2. Separate Functions to Other c/h Files
 * 3. Separate Files
 * */

int sockfd;

void cleanExit(){
    close(sockfd);
    exit(0);
}

int main(int argc, char **argv){
    signal(SIGTERM, cleanExit);
    signal(SIGINT, cleanExit);

    open();
    return 0;
}

void open(){
    int clientfd;
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    int size, option;

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, &my_addr, sizeof(my_addr)) == -1){
        perror("bind");
        exit(1);
    }
    if(listen(sockfd, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }
    printf("Clear preparing.\n");
    while(1){
        size = sizeof(client_addr);
        if((clientfd = accept(sockfd, &client_addr, &size)) == -1){
            perror("accept");
            continue;
        }
        
        if( fork() == 0){
            part2(clientfd);
            printf("\nClosed\n");

            fflush(stdout);
            shutdown(clientfd, SHUT_RDWR);
            close(clientfd);
            exit(0);
        }
    }
}

void part1(int clientfd){
    char buff[BUFF_SIZE];
    while(read(clientfd, buff, sizeof(buff)) > 0){
        printf("%s", buff);
    }
}

void part2(int clientfd){
    request req;
    response res;

    decodeRequest(clientfd, &req);
    // printf("GET %s %s\n", req.filename, req.version);
    sendResponse(clientfd, &req);
}