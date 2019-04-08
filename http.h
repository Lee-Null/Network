#ifndef BUFF
    #define BUFF_SIZE 256
    #define BUFF
#endif

#define SIZE 256
#define TYPE_SIZE 64

typedef enum {POST, GET} method;

typedef struct {
    method mtd;
    char version[SIZE];
    char filename[SIZE];
} request;

typedef struct {
    request *req;
    int code;
    int contentLength;
    char type[TYPE_SIZE];
} response;

int readLine(int fd, char *buff, int size);

void sendResponse(int clientfd, request *req);
int decodeRequest(int clientfd, request *req);
void encodeResponse(response* result, char *data);

void extToTypeString(char *ext, char *type);