
enum ContentType {
    NONE,
    TEXT_HTML,
    APPLICATION_JSON
};

enum HTTPMethod {
    GET=1,
    METHOD_NOT_IMPLEMENTED
};

struct HTTPHeader {
    char* name;
    char* value;
    void* next;
};

struct HTTPRequest {
    enum HTTPMethod method;
    char* path;   
    struct HTTPHeader* headers;
    char* body;
};

struct HTTPResponse {
    int code;
    enum ContentType contentType;
    int contentLength;
    char* content;
};


extern int startServer(char* ip, int port);
extern void addMapping(char path[], struct HTTPResponse* (*handler)(struct HTTPRequest* request));