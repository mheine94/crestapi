#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libhttpserver.h"
#include "libcjson.h"

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

struct HTTPResponse* handleHelloJson(struct HTTPRequest* request){
    struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
    res->code = 200;
    res->contentType = APPLICATION_JSON;

    
    JSON* json = CJSON.newObj();
    char wor[] = "world";
    char* worp = malloc(strlen(wor)*sizeof(char));
    strcpy(worp, (char*) wor);
    CJSON.objSet(json, "hello", CJSON.newStringValue(worp));
    CJSON.objSet(json, "raining", CJSON.newBooleanValue(1));
    JSON* array = CJSON.newArrayValue();
    CJSON.arrayPush(array, CJSON.newIntValue(1));
    CJSON.arrayPush(array, CJSON.newBooleanValue(0));
    char arrstr[] = "a";
    char* w2 = malloc(strlen(arrstr)*sizeof(char));
    strcpy(w2, (char*) arrstr);
    CJSON.arrayPush(array, CJSON.newStringValue(w2));
    CJSON.arrayPush(array, CJSON.newNullValue());
    CJSON.arrayPush(array, CJSON.newArrayValue());
    JSON* array2 = CJSON.newArrayValue();
    CJSON.arrayPush(array2, CJSON.newBooleanValue(0));
    CJSON.arrayPush(array, array2);

    CJSON.objSet(json, "myarray", array);
    
    char version[] = "version";
    char* vp = malloc(strlen(version)*sizeof(char));
    strcpy(vp, (char*) version);
    CJSON.objSet(json, vp , CJSON.newIntValue(1));
    char* jsonString = CJSON.stringify(json);
    CJSON.free(json);


    res->content = jsonString;
    res->contentLength = strlen(res->content);

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

struct HTTPResponse* handleEcho(struct HTTPRequest* request){
    struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
    res->code = 200;
    res->contentType = APPLICATION_JSON;

    JSON* json = CJSON.newObj();

    if(request->headers != NULL){
        JSON* headers = CJSON.newObj();
        struct HTTPHeader* header = request->headers;
        while(1){
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
    char * string = CJSON.stringify(json);
    res->content = string;
    res->contentLength = strlen(string); 
    CJSON.free(json);
    return res;
}

int main(){
    addMapping("/", handleRoot);
    addMapping("/test", handleTest);
    addMapping("/hello", handleHelloJson);
    addMapping("/echo", handleEcho);
    
    startServer("127.0.0.1", 8080);
}