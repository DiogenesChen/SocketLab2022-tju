#include "cgi.h"

char CONTEN_LENGTH[8] = "";
char CONTENT_TYPE[32] = "";
char GATEWAY_INTERFACE[] = "CGI/1.1";
char PATH_INFO[64] = "";
char QUERY_STRING[64] = "";
char REMOTE_ADDR[64] = "";
char REQUEST_METHOD[64] = "";
char REQUEST_URI[64] = "";
char SCRIPT_NAME[64] = "cgi-bin/login";
char SERVER_PORT[64] = "";
char SERVER_PROTOCOL[] = "HTTP/1.1";
char SERVER_SOFTWARE[] = "Liso/1.0";
char HTTP_ACCEPT[156] = "";
char HTTP_REFERER[64] = "";
char HTTP_ACCEPT_ENCODING[64] = "";
char HTTP_ACCEPT_LANGUAGE[64] = "";
char HTTP_ACCEPT_CHARSET[64] = "";
char HTTP_HOST[64] = "";
char HTTP_COOKIE[64] = "";
char HTTP_USER_AGENT[256] = "";
char HTTP_CONNECTION[64] = "";
char username[64] = "";
char passwd[64] = "";

char* cgi_login(char* buf, int readret, int client_sock, Request *request)
{
    int BUF_SIZE = 998192;
    char* buffer = (char *)malloc(BUF_SIZE);
    strcpy(buffer, buf);
    int pos;
    for (int i = 0; i < strlen(request->http_uri); i++) {
        if (request->http_uri[i] == '?') {
            pos = i;
            break;
        }
    }
    char* p = strstr(request->http_uri + pos, "username") + strlen("username") + 1;
    //从URI中进行username寻位
    char* q = strstr(request->http_uri + pos, "passwd") + strlen("passwd") + 1;
    //从URI中进行passwd寻位

    if (strstr(request->http_uri + pos, "username") != NULL && strstr(request->http_uri + pos, "passwd") != NULL)
    {
        int offset = 0;
        int len_u = 0;
        int len_p = 0;
        while (!(p[offset] == '&' || p[offset] == 0)) {
            username[len_u++] = p[offset++];
        }
        offset = 0;
        while (!(q[offset] == '&' || q[offset] == 0)) {
            passwd[len_p++] = q[offset++];
        }
        //对于URI中的用户名及密码进行读取
    

        strncpy(PATH_INFO, request->http_uri, pos);
        strcpy(QUERY_STRING, request->http_uri + pos + 1);
        strcpy(REMOTE_ADDR, "127.0.0.1");
        strcpy(REQUEST_METHOD, "CGI_GET");
        sprintf(SERVER_PORT, "%d", 9999);

        for (int i = 0; i < request->header_count; ++i) {
            if (strcmp(request->headers[i].header_name, "Accept") == 0) {
                strcpy(HTTP_ACCEPT, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "Referer") == 0) {
                strcpy(HTTP_REFERER, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "Accept-Encoding") == 0) {
                strcpy(HTTP_ACCEPT_ENCODING, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "Accept-Language") == 0) {
                strcpy(HTTP_ACCEPT_LANGUAGE, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "Accept-Charset") == 0) {
                strcpy(HTTP_ACCEPT_CHARSET, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "Host") == 0) {
                strcpy(HTTP_HOST, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "Cookie") == 0) {
                strcpy(HTTP_COOKIE, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "User-Agent") == 0) {
                strcpy(HTTP_USER_AGENT, request->headers[i].header_value);
            }
            if (strcmp(request->headers[i].header_name, "Connection") == 0) {
                strcpy(HTTP_CONNECTION, request->headers[i].header_value);
            }
        } 

        int fd_in = open("./static_site/Login_OK.html", 0x0002);
        memset(buffer, 0, BUF_SIZE);
        read(fd_in,buffer,8192);

        char* alterBuf = (char *)malloc(8192);
        sprintf(alterBuf, "HTTP/1.1 200 OK\r\n\r\n");
        strcat(alterBuf, buffer);
        strcpy(buffer, alterBuf);  

        return buffer;  
    }
    int fd_in = open("./static_site/Login.html", 0x0002);
    memset(buffer, 0, BUF_SIZE);
    read(fd_in,buffer,8192);

    char* alterBuf = (char *)malloc(8192);
    sprintf(alterBuf, "HTTP/1.1 200 OK\r\n\r\n");
    strcat(alterBuf, buffer);
    strcpy(buffer, alterBuf);

    return buffer;
}