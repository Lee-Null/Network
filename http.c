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

#ifndef HTTP
    #define HTTP
    #include "http.h"
#endif

#define READOUT 10

//Read each line of requests
int readLine(int fd, char *buff, int size){
    int i = 0;
    char ch = 0;
    while(read(fd, &ch, sizeof(ch)) > 0){
        buff[i] = ch;
        if(ch == '\n')
            break;
        i++;
    }
    buff[i-1] = '\0';
    printf("%s\n", buff);
    return i;
}

//Decode request header.
int decodeRequest(int clientfd, request *req){
    char buff[BUFF_SIZE];
    char *ptr; int readSize;
    while(1){
        readSize = readLine(clientfd, buff, sizeof(buff));
        if(strlen(buff) == 0 || readSize == 0) break;

        if(strstr(buff, "GET") != NULL){
            // Decode GET Request
            req -> mtd = GET;

            ptr = strtok(buff, " "); // GET
            ptr = strtok(NULL, " "); // Filename
            strcpy(req -> filename, ptr);
            ptr = strtok(NULL, " "); // HTTP Version
            strcpy(req -> version, ptr);
        }
    }
    return 0;
}

//Send response to client.
void sendResponse(int clientfd, request *req){
    FILE *fp = 0;
    response res;
    char header[SIZE], *cache;
    unsigned char buff[BUFF_SIZE];
    int readSize;

    //Initialize response struct
    res.req = req;
    res.contentLength = 0;
    res.code = 0;
    res.type[0] = '\0';

    //File Open
    fp = fopen(res.req -> filename + 1, "rb");
    if(fp == NULL){
        //NO FILE is in Directory : 404
        res.code = 404;
        sprintf(header, "%s %d Not Found\r\n\r\n<h1>404 NOT FOUND %s</h1>", res.req -> version, res.code, res.req -> filename);

        //Write Response
        write(clientfd, header, strlen(header));
    }
    else{
        //FILE is in Directory    : 200
        res.code = 200;

        //Get file size
        fseek(fp, 0L, SEEK_END);
        res.contentLength = ftell(fp);
        rewind(fp);

        //Encode response to header from filename and request information
        cache = strstr(res.req -> filename, ".");
        if(cache != NULL)
            extToTypeString(cache+1, res.type);
        encodeResponse(&res, header);

        // Write HTTP Header
        write(clientfd, header, strlen(header));

        // Write HTTP Data of File
        while((readSize = fread(buff, sizeof(char), sizeof(buff),fp)) > 0)
            write(clientfd, buff, readSize);

        //Close and Flush file
        fflush(fp);
        fclose(fp);
    }
}

//Encode response to data.
void encodeResponse(response* res, char *data){
    //Declaration for time
    time_t rawtime;
    struct tm *timeinfo;
    char date[128];

    // Get Time
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(date, 128, "%a, %d %b %Y %X %Z", timeinfo);

    //Encode response to header
    sprintf(data, "%s %d OK\r\n", res -> req -> version, res -> code);
    sprintf(data, "%sDate: %s\r\n", data, date);
    sprintf(data, "%sContent-Length: %d\r\n", data, res -> contentLength);
    sprintf(data, "%sContent-Type: %s\r\n\r\n", data, res -> type);
}

// Change Extension to the string of content-type.
void extToTypeString(char *ext, char *type){
    if(strstr(ext, "png") != NULL)
        strcpy(type, "image/png");

    else if(strstr(ext, "mpeg") != NULL)
        strcpy(type, "video/mpeg");
    
    else if(strstr(ext, "jpg") != NULL || strstr(ext, "jpeg") != NULL)
        strcpy(type, "image/jpeg");
    
    else if(strstr(ext, "css") != NULL)
        strcpy(type, "text/css");
    
    else if(strstr(ext, "epub") != NULL)
        strcpy(type, "application/epub+zip");
    
    else if(strstr(ext, "html") != NULL || strstr(ext, "htm") != NULL)
        strcpy(type, "text/html");
    
    else if(strstr(ext, "pdf") != NULL)
        strcpy(type, "application/pdf");
    
    else if(strstr(ext, "json") != NULL)
        strcpy(type, "application/json");
    
    else if(strstr(ext, "ppt") != NULL)
        strcpy(type, "application/vnd.ms-powerpoint");
    
    else if(strstr(ext, "doc") != NULL)
        strcpy(type, "application/msword");
    
    else if(strstr(ext, "xml") != NULL)
        strcpy(type, "application/xml");
    
    else if(strstr(ext, "zip") != NULL)
        strcpy(type, "application/zip");
    
    else if(strstr(ext, "avi") != NULL)
        strcpy(type, "video/x-msvideo");
    
    else
        strcpy(type, "application/octet-stream");
}
