#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libhttpserver.h"
#include "libcjson.h"


char* copyString(int start, int end, char* source){
    int len = end - start;
    char* string = malloc(sizeof(char) * (len+1));
    for(int i = 0; start + i < end; i++){
        string[i] = source[start+i];
    } 
    string[len] = '\0';
    return string;
}

struct HTTPResponse* handleRoot(struct HTTPRequest* request){
    struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
    res->code = 200;
    res->contentType = TEXT_HTML;
    char contentString[] = "Hello World!";
    res->contentLength = strlen(contentString);
    res->content = copyString(0, strlen(contentString), contentString);

    return res;
}



struct HTTPResponse* handleEcho(struct HTTPRequest* request){
    struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
    res->code = 200;
    res->contentType = APPLICATION_JSON;

    JSON* json = CJSON.newObj();

    switch (request->method){
        case GET:
            CJSON.objSet(json, "method", CJSON.newStringValue(copyString(0, 4, "GET")));
            break; 
        default:
            CJSON.objSet(json, "method", CJSON.newStringValue(copyString(0, 12, "unsupported")));
            break;
    }
    CJSON.objSet(json, "path", CJSON.newStringValue(request->path));

    if(request->headers != NULL){
        JSON* headers = CJSON.newObj();
        struct HTTPHeader* header = request->headers;

        while(1){
            //printf("HEADERNAME: \'%s\' HEADERVALUE: \'%s\'\n", header->name, header->value);
            CJSON.objSet(headers, header->name, CJSON.newStringValue(header->value));
            if(header->next == NULL){
                break;
            }
            header = header->next;
        }

        CJSON.objSet(json, "headers", headers);
    } else {
        CJSON.objSet(json, "headers", CJSON.newNullValue());
    }
    
    if(request->body != NULL){
        CJSON.objSet(json, "body", CJSON.newStringValue(request->body));
    } else {
        CJSON.objSet(json, "body", CJSON.newNullValue());
    }

    char * string = CJSON.stringify(json);
    res->content = string;
    res->contentLength = strlen(string); 
    CJSON.free(json);
    return res;
}

struct HTTPResponse* handleEchoParse(struct HTTPRequest* request){
    struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
    res->code = 200;
    res->contentType = APPLICATION_JSON;

    JSON* json = CJSON.parse(request->body);
    char* string = CJSON.stringify(json);
    res->content = string;
    res->contentLength = strlen(string); 
    CJSON.free(json);
    return res;
}

int main(){
    addMapping("/", handleRoot);
    addMapping("/echo", handleEcho);
    addMapping("/parse", handleEchoParse);    
    startServer("127.0.0.1", 8080);
}