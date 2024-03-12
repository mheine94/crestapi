#include <stdlib.h>
#include <string.h>
#include "httpserver.h"

struct HTTPResponse* handleRoot(struct HTTPRequest* request){
    struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
    res->code = 200;
    res->contentType = TEXT_HTML;
    char contentString[] = "Hello World!";
    res->contentLength = strlen(contentString);
    res->content = malloc(sizeof(char)* strlen(contentString));
    strcpy(res->content, contentString);

    return res;
}

struct HTTPResponse* handleTest(struct HTTPRequest* request){
    struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
    res->code = 200;
    res->contentType = APPLICATION_JSON;
    char contentString[] = "{ \"hello\": \"world\"}";
    res->contentLength = strlen(contentString);
    res->content = malloc(sizeof(char)* strlen(contentString));
    strcpy(res->content, contentString);

    return res;
}

int main(){
    addMapping("/", handleRoot);
    addMapping("/test", handleTest);
    
    startServer("127.0.0.1", 8080);
}