#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "libhttpserver.h"


struct RouteHandler {
    struct RouteHandler* next;
    char* route;
    struct HTTPResponse* (*handler)(struct HTTPRequest*);
};

struct RouteHandler* handlers = NULL;
int runServer = 0;

char* getResponse(char* request);

int initWindowsSockets(){
    //-----------------------------------------
    // Declare and initialize variables
    WSADATA wsaData = {0};
    int iResult = 0;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wprintf(L"WSAStartup failed: %d\n", iResult);
        return 1;
    }
}

int cleanupWindowSockets(){
    WSACleanup();
}

int makeSocket(SOCKET* sock){
    *sock = INVALID_SOCKET;
    int iFamily = AF_INET;
    int iType = SOCK_STREAM;
    int iProtocol = IPPROTO_TCP;

    wprintf(L"Calling socket with following parameters:\n");
    wprintf(L"  Address Family = ");
    switch (iFamily) {
    case AF_UNSPEC:
        wprintf(L"Unspecified");
        break;
    case AF_INET:
        wprintf(L"AF_INET (IPv4)");
        break;
    case AF_INET6:
        wprintf(L"AF_INET6 (IPv6)");
        break;
    case AF_NETBIOS:
        wprintf(L"AF_NETBIOS (NetBIOS)");
        break;
    case AF_BTH:
        wprintf(L"AF_BTH (Bluetooth)");
        break;
    default:
        wprintf(L"Other");
        break;
    }
    wprintf(L" (%d)\n", iFamily);
    
    wprintf(L"  Socket type = ");
    switch (iType) {
    case 0:
        wprintf(L"Unspecified");
        break;
    case SOCK_STREAM:
        wprintf(L"SOCK_STREAM (stream)");
        break;
    case SOCK_DGRAM:
        wprintf(L"SOCK_DGRAM (datagram)");
        break;
    case SOCK_RAW:
        wprintf(L"SOCK_RAW (raw)");
        break;
    case SOCK_RDM:
        wprintf(L"SOCK_RDM (reliable message datagram)");
        break;
    case SOCK_SEQPACKET:
        wprintf(L"SOCK_SEQPACKET (pseudo-stream packet)");
        break;
    default:
        wprintf(L"Other");
        break;
    }
    wprintf(L" (%d)\n", iType);

    wprintf(L"  Protocol = %d = ", iProtocol);
    switch (iProtocol) {
    case 0:
        wprintf(L"Unspecified");
        break;
    case IPPROTO_ICMP:
        wprintf(L"IPPROTO_ICMP (ICMP)");
        break;
    case IPPROTO_IGMP:
        wprintf(L"IPPROTO_IGMP (IGMP)");
        break;
    case IPPROTO_TCP:
        wprintf(L"IPPROTO_TCP (TCP)");
        break;
    case IPPROTO_UDP:
        wprintf(L"IPPROTO_UDP (UDP)");
        break;
    case IPPROTO_ICMPV6:
        wprintf(L"IPPROTO_ICMPV6 (ICMP Version 6)");
        break;
    default:
        wprintf(L"Other");
        break;
    }
    wprintf(L" (%d)\n", iProtocol);

    *sock = socket(iFamily, iType, iProtocol);
    if (*sock == INVALID_SOCKET) {
        wprintf(L"socket function failed with error = %d\n", WSAGetLastError() );
        return 1;
    } else {
        wprintf(L"socket function succeeded\n");
        return 0;
        // Close the socket to release the resources associated
        // Normally an application calls shutdown() before closesocket 
        //   to  disables sends or receives on a socket first
        // This isn't needed in this simple sample
        ///iResult = closesocket(sock);
        //if (iResult == SOCKET_ERROR) {
        //    wprintf(L"closesocket failed with error = %d\n", WSAGetLastError() );
        //    WSACleanup();
        //   return 1;
        //}    
    }
}

int bindToAdress(SOCKET soc, char* ip, int port){
    // The socket address to be passed to bind
    SOCKADDR_IN service;
        // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ip);
    service.sin_port = htons(port);

    //----------------------
    // Bind the socket.
    int rc = bind(soc, (SOCKADDR *) &service, sizeof (service));
    if (rc == SOCKET_ERROR) {
        wprintf(L"bind failed with error %u\n", WSAGetLastError());
        closesocket(rc);
        WSACleanup();
        return 1;
    }
    printf("Bound socket to %s on port %d\n", ip, port);
    return 0;
}

int listenOnSocket(SOCKET soc){    
    //----------------------
    // Listen for incoming connection requests 
    // on the created socket
    if (listen(soc, SOMAXCONN) == SOCKET_ERROR){
        wprintf(L"listen function failed with error: %d\n", WSAGetLastError());
        return 1;
    }
    wprintf(L"Listening on socket...\n");
    return 0;
}

int serializeResponse(struct HTTPResponse* res, char* buf){
    int position = 0;
    char http[] = "HTTP/1.0 ";
    strcpy(buf, http);

    position += strlen(http);
    int codeLen = snprintf(NULL, 0, "%d", res->code);
    sprintf(&buf[position], "%d", res->code);
    position += codeLen;
    buf[position++] = ' ';
    char desc[] = "CRestapi";
    strcpy(&buf[position], desc);
    position += strlen((char*) &desc);
    buf[position++] = '\n';

    char contLen[] = "Content-Length: ";
    strcpy(&buf[position], contLen);
    position += strlen((char*) &contLen);
    
    if(res->contentType != NONE){
        int contLenNumLen = snprintf(NULL, 0, "%d", res->contentLength);
        sprintf(&buf[position], "%d", res->contentLength);
        position +=contLenNumLen;
    }else{
        buf[position++] = '0';
    }
   
    buf[position++] = '\n';
    
    char contType[] = "Content-Type: ";
    strcpy(&buf[position], contType);
    position += strlen(contType);

    switch(res->contentType){
        case TEXT_HTML:
            char html[] =  "text/html";
            strcpy(&buf[position], html);
            position += strlen(html); 
        break;
        case APPLICATION_JSON:
            char json[] = "application/json";
            strcpy(&buf[position],json);
            position += strlen(json); 
        break;
        default:
            char none[] = "none";
            strcpy(&buf[position],none);
            position += strlen(none); 
        break;
    }
    
    buf[position++] = '\n';
    buf[position++] = '\n';
    if(res->contentType != NONE){
        for(int i = 0; i < res->contentLength; i++){
            buf[position+i] = res->content[i];
        }
        position += res->contentLength;
    }
    buf[position++] = '\n';

    return position;
} 

struct HTTPRequest* parseRequest(char* request){
    printf("Request was\n%s\n\n", request);
    struct HTTPRequest* httpRequest = malloc(sizeof(struct HTTPRequest));
    httpRequest->headers = NULL;
    httpRequest->body = NULL;

    int i;
    for(i = 0; i < 10 && request[i] != '\0'; i ++){
        if(request[i] == ' '){
            printf("%.*s ", i, request);
            if(request[0] == 'G'){
                httpRequest->method = GET;
            }else{
                httpRequest->method = METHOD_NOT_IMPLEMENTED;
            } 
            break;
        }
    }
    
    int j;
    for(j = i+1; j < 1000 && request[j] != '\0'; j++){
        if(request[j] == ' '){
            int pathLen =  j-(i+1);
            httpRequest->path = malloc(sizeof(char) * (pathLen+1));
            for(int k = 0; k < pathLen; k++){
                httpRequest->path[k] = request[i+1+k];
            }
            httpRequest->path[pathLen] = '\0';
            
            printf("%s\n", httpRequest->path);
            break;
        }
    }
    //printf("_____PATH___PARSED_____\n");
    for(; j < 1000 && request[j] != '\0'; j++){
        if(request[j] == '\n'){
            //printf("newline ffound\n");
            j++;
            break;
        }
    }
    //printf("_____HEADER_START_FOUND_____\n");
    int stringStart = j;
    char* headerName = NULL;
    char* headerValue = NULL;
    int parseHeaderState = 0;

    struct HTTPHeader* current = NULL;

    for(; j < 1000 && request[j] != '\0'; j++){
        printf("%c", request[j]);
        if(parseHeaderState == 0 && request[j] != '\n'){
            stringStart = j;
            //printf("found header name start %d\n", j);
            parseHeaderState = 1;
        } else if(parseHeaderState == 0 && request[j] == '\n'){
            break;
        } else if(parseHeaderState == 1 && request[j] != ':'){
           
        } else if(parseHeaderState == 1 && request[j] == ':'){
            //printf("\n");
            //printf("_____FOUND____NAME______\n");
            int len = (j) - stringStart;
            headerName = malloc(sizeof(char) * (len+1));
            for(int k = 0; stringStart + k < j; k++){
                headerName[k] = request[stringStart+k];
            } 
            headerName[len] = '\0';
            //printf("Header Name: \'%s\'\n", headerName);
            parseHeaderState = 2;
        } else if(parseHeaderState == 2 && request[j] == ' ') {
            //printf("_____FOUND SPACE____\n");
            stringStart = j+1;
            parseHeaderState = 3;
        } else if(parseHeaderState == 3 && request[j] == '\n'){
            int len = j - stringStart;
            headerValue = malloc(sizeof(char) * (len+1));
            for(int k = 0; stringStart + k < j; k++){
               headerValue[k] = request[stringStart+k];
            } 
            headerValue[len] = '\0';

            //printf("_____FOUND____VALUE_____\n");
            //printf("Header Value: \'%s\'\n", headerValue);
            struct HTTPHeader* nextHeader = malloc(sizeof(struct HTTPHeader));
            nextHeader->name = headerName;
            nextHeader->value = headerValue;
            nextHeader->next = NULL;
            

            if(httpRequest->headers == NULL){
               httpRequest->headers = nextHeader;
               current = nextHeader;
            }else{
               current->next = nextHeader;
               current = nextHeader;
            }
            parseHeaderState = 0;
        }
    }
    return httpRequest;
}

int read(SOCKET soc, char* buffer){
    //----------------------
    wprintf(L"Waiting for client to connect...\n");
    SOCKET acceptSocket = INVALID_SOCKET;
    //----------------------
    // Accept the connection.
    acceptSocket = accept(soc, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET) {
        wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
        closesocket(soc);
        WSACleanup();
        return 1;
    } else{
        wprintf(L"Client connected.\n");
        
        int totalBytesRead = 0;
        int bytesRead = 0;
        do {
            bytesRead = recv(acceptSocket, buffer, 5000, 0);
            if(bytesRead == 0){
                printf("Connection closed\n");
            }else if(bytesRead > 0){
                printf("Read %d bytes..\n", bytesRead);
            }else {
                printf("Error\n");
            }
            totalBytesRead += bytesRead;
        } while(1 ==0  && bytesRead > 0);
        buffer[bytesRead] = '\0';
        char* request = malloc(sizeof(char) * (bytesRead+1));
        strcpy(request, buffer);
        struct HTTPRequest* parsedRequest = parseRequest(request);
        /*printf("_____HEADERS_____\n");
        struct HTTPHeader* header = parsedRequest->headers;
        while(1){
            printf("%s: %s\n", header->name, header->value);
            if(header->next == NULL){
                break;
            }
            header = header->next;
        }
        
        printf("_____HEADERS__END___\n");
        */
        struct HTTPResponse* response;

        int foundHandler = 0;
        struct RouteHandler* current = handlers;
        while(current != NULL){
            if(strcmp(parsedRequest->path, current->route) == 0){
                printf("Found handler for path \"%s\"\n", parsedRequest->path);
                printf("Handler path \"%s\"\n", current->route);
                response = current->handler(parsedRequest);
                break;
            }else if(current->next != NULL){
                printf("Check next handler\n");
                current = current->next;
            }else {
                printf("No handler found for path %s\n", parsedRequest->path);
                response = malloc(sizeof(struct HTTPResponse));
                response->code = 404;
                response->contentLength = 0;
                response->contentType = NONE;
                response->content = NULL;   
                break;
            }
        }
        char buf[5000];
        int len = serializeResponse(response, (char *) &buf);
        printf("Responded with:\n%.*s", len, (char*) &buf);
        int rc = send(acceptSocket, (char*) &buf, len, 0);
        closesocket(acceptSocket);
        
        free(response->content);
        free(response);
        free(request);
        return 0;
    }
}

void addMapping(char path[], struct HTTPResponse* (*handler)(struct HTTPRequest* request)){
    struct RouteHandler* newHandler = malloc(sizeof(struct RouteHandler));
    newHandler->route = malloc(sizeof(char) * (strlen(path)+1));
    strcpy(newHandler->route, path);
    newHandler->route[strlen(path)] = '\0';
    newHandler->handler = handler;
    printf("Registered handler for path \"%s\"\n", newHandler->route);
    newHandler->next = NULL;

    if(handlers == NULL){
        handlers = newHandler;        
    }else{
        struct RouteHandler* current = handlers;
        while (current != NULL)
        {
            if(current->next == NULL){
                current->next = newHandler;
                return;
            }else {
                current = current->next;
            }
        }
    }
}

int startServer(char* ip, int port){
    runServer = 1;
    initWindowsSockets(); 
    SOCKET soc;
    int rc = makeSocket(&soc);
    rc = bindToAdress(soc, "127.0.0.1", 8080);
    if(rc == 0){
        rc = listenOnSocket(soc);
        if(rc == 0){
            char* buffer = malloc(sizeof(char) * 5000);
            while(1){
                for(int i = 0; i< 5000; i++){
                    buffer[i] = '\0';
                }
                rc = read(soc, buffer);
            }
            free(buffer);
        }

        rc = closesocket(soc);
        if (rc == SOCKET_ERROR) {
            wprintf(L"closesocket function failed with error %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
    }else{
        wprintf(L"bind returned success\n");
    }
    cleanupWindowSockets();
}

/*

int main(){

    addMapping("/", handleRoot);
    addMapping("/test", handleTest);

    initWindowsSockets(); 
    
    startServer("127.0.0.1", 8080);

    cleanupWindowSockets();

    return 0;
}

*/