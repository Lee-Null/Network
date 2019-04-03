#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

// TODO : binary file control!

#ifndef HTTP
    #define HTTP
    #include "http.h"
#endif

#define READOUT 10

int read_line(int fd, char *buff, int size){
    int i = 0, readSize = 0;
    char ch = 0;
    // for(i = 0; i < size-1;i++){
    //     cache = read(fd, &ch, sizeof(ch));
    //     if(cache == -1)
    //         return -1;
    //     if(cache == 0)
    //         break;
    while(read(fd, &ch, sizeof(ch)) > 0){
        buff[i] = ch;
        if(ch == '\n')
            break;
        readSize++;
        i++;
    }
    buff[--i] = '\0';
    printf("%s\n", buff);
    return readSize;
}

int decodeRequest(int clientfd, request *req){
    char buff[BUFF_SIZE];
    char *ptr; int readSize;
    while(1){
        readSize = read_line(clientfd, buff, sizeof(buff));
        printf("READ\n");
        if(strlen(buff) == 0 || readSize == 0) break;

        if(strstr(buff, "GET") != NULL){
            req -> mtd = GET;

            ptr = strtok(buff, " ");
            ptr = strtok(NULL, " ");
            strcpy(req -> filename, ptr);
            ptr = strtok(NULL, " ");
            strcpy(req -> version, ptr);
        }
    }
    printf("\nDECODED\n");
    return 0;
}

void sendResponse(int clientfd, request *req){
    FILE *fp = 0;
    response res;
    char text[SIZE];
    unsigned char buff[BUFF_SIZE];
    char ch;
    int readSize;
    char *cache;

    res.req = req;
    fp = fopen(res.req -> filename + 1, "rb");
    if(fp == NULL){
        //NO FILE
        res.code = 404;
        sprintf(text, "%s %d Not Found\r\nContent-Length", res.req -> version, res.code);
        printf("resp : %s\n", text);
        write(clientfd, text, strlen(text));
    }
    else{
        //FILE
        res.code = 200;
        fseek(fp, 0L, SEEK_END);
        res.contentLength = ftell(fp);
        rewind(fp);

        cache = strstr(res.req -> filename, ".");
        if(cache != NULL)
            extToTypeString(cache+1, res.type);
        encodeResponse(&res, text);
        // printf("%s\n", res.type);

        printf("%s", text);

        write(clientfd, text, strlen(text));
        // write(clientfd, "\r\n", strlen("\r\n"));

        int count = 0;
        // printf("\nFD : %d\n", clientfd);
        while((readSize = fread(buff, sizeof(char), sizeof(buff),fp)) > 0){
            write(clientfd, buff, readSize);
            // write(stdout, buff, readSize);
            count += readSize;
        }
        // printf("SENDED : %d\n", count);

        // while(!feof(fp)){
        //     fgets(buff, BUFF_SIZE, fp);
        //     write(clientfd, buff, strlen(buff));
        // }

        // while((ch = fgetc(fp)) != EOF){
        //     write(clientfd, ch, sizeof(ch));
        //     printf("%c", ch);
        // }

        // cache = malloc(sizeof(char) * (res.contentLength + 1));
        // fread(cache, sizeof(char), res.contentLength + 1, fp);

        // printf("%s", cache);

        // write(clientfd, cache, strlen(cache));

        // write(clientfd, "\0", sizeof(char));

        fflush(fp);
        fclose(fp);
    }
}

void encodeResponse(response* res, char *data){
    time_t rawtime;
    struct tm *timeinfo;
    char date[128];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(date, 128, "%a, %d %b %Y %X %Z", timeinfo);
    // printf("%s\n", date);

    sprintf(data, "%s %d OK\r\n", res -> req -> version, res -> code);
    sprintf(data, "%sDate: %s\r\n", data, date);
    sprintf(data, "%sContent-Length: %d\r\n", data, res -> contentLength);
    sprintf(data, "%sContent-Type: %s\r\n\r\n", data, res -> type);
}

void extToTypeString(char *ext, char *type){
    if(strstr(ext, "png") != NULL){
        strcpy(type, "image/png");
    }
    else if(strstr(ext, "mpeg") != NULL){
        strcpy(type, "video/mpeg");
    }
    else if(strstr(ext, "jpg") != NULL || strstr(ext, "jpeg") != NULL){
        strcpy(type, "image/jpeg");
    }
    else if(strstr(ext, "css") != NULL){
        strcpy(type, "text/css");
    }
    else if(strstr(ext, "epub") != NULL){
        strcpy(type, "application/epub+zip");
    }
    else if(strstr(ext, "html") != NULL || strstr(ext, "htm") != NULL){
        strcpy(type, "text/html");
    }
    else if(strstr(ext, "pdf") != NULL){
        strcpy(type, "application/pdf");
    }
    else if(strstr(ext, "json") != NULL){
        strcpy(type, "application/json");
    }
    else if(strstr(ext, "ppt") != NULL){
        strcpy(type, "application/vnd.ms-powerpoint");
    }
    else if(strstr(ext, "doc") != NULL){
        strcpy(type, "application/msword");
    }
    else if(strstr(ext, "xml") != NULL){
        strcpy(type, "application/xml");
    }
    else if(strstr(ext, "zip") != NULL){
        strcpy(type, "application/zip");
    }
    else if(strstr(ext, "avi") != NULL){
        strcpy(type, "video/x-msvideo");
    }
    else{
        strcpy(type, "application/octet-stream");
    }
}
