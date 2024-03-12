
enum ContentType {
    NONE,
    TEXT_HTML,
    APPLICATION_JSON
};

enum HTTPMethod {
    GET=1,
    METHOD_NOT_IMPLEMENTED
};

struct HTTPRequest {
    enum HTTPMethod method;
    char* path;   
};

struct HTTPResponse {
    int code;
    enum ContentType contentType;
    int contentLength;
    char* content;
};


extern int startServer(char* ip, int port);
extern void addMapping(char path[], struct HTTPResponse* (*handler)(struct HTTPRequest* request));