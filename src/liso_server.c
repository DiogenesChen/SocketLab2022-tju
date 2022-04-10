/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef _PARSE_H
#define _PARSE_H

#include "parse.h"

#endif

#include "log.h"

#ifndef _CGI_H
#define _CGI_H

#include "cgi.h"

#endif

#define ECHO_PORT 9999
#define BUF_SIZE 998192

extern Request* parse(char *buffer, int size,int socketFd);
extern char* cgi_login(char* buffer, int readret, int client_sock, Request *request);

extern void log_write(char* msg, int length, int statCode);
extern void error_write(char* msg);

int AcaBUF_SIZE = BUF_SIZE;

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

char* request_handler(char* buffer, int readret, int client_sock)
{
    Request *request = (Request *) malloc(sizeof(Request));
    char* buf = (char *)malloc(BUF_SIZE);
    strcpy(buf, buffer);
    fprintf(stderr, "%s\n",buf);
    request = parse(buf, BUF_SIZE, client_sock);
    memset(buf, 0, BUF_SIZE);
    if (request == NULL || readret == BUF_SIZE)
    {
        char resp[64] = "HTTP/1.1 400 Bad request\r\n\r\n";
        strcpy(buf, resp);
        error_write("Bad request");
    }//Bad request

    else if (strcmp(request->http_method, "GET") && strcmp(request->http_method, "HEAD") && strcmp(request->http_method, "POST"))
    {
        char resp[64] = "HTTP/1.1 501 Not Implemented\r\n\r\n";
        strcpy(buf, resp);
        error_write("Not Implemented");
    }//not implemented

    else if (strcmp(request->http_version, "HTTP/1.1"))
    {
        char resp[64] = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
        strcpy(buf, resp);
        error_write("HTTP Version not supported");
    }//Version not supported

    else if (!strcmp(request->http_method, "HEAD"))
    {
        char resp[64] = "HTTP/1.1 200 OK\r\n\r\n";
        strcpy(buf, resp);
        char requestLine[64];
        sprintf(requestLine, "%s %s %s", request->http_method, request->http_uri, request->http_version);
        log_write(requestLine, 0, 200);
    }//Head method

    else if (!strcmp(request->http_method, "GET"))
    {
        char fileAddr[64] = "./static_site";

        if(strstr(request->http_uri, "/cgi-bin/"))
        {
            if (strstr(request->http_uri, "/cgi-bin/login"))
            {
                char requestLine[64];
                sprintf(requestLine, "%s %s %s", request->http_method, request->http_uri, request->http_version);
                log_write(requestLine, 0, 200);
                memset(buf, 0, BUF_SIZE);
                buf = cgi_login(buffer, readret, client_sock, request);
            }

            else 
            {
                char resp[64] = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                strcpy(buf, resp);
                error_write("Internal Server Error");
            }
        }
        //CGI程序. CGI的URI入口在/cgi-bin/下
        else 
        {
            if (!strcmp(request->http_uri, "/")) strcat(fileAddr, "/index.html");
            else strcat(fileAddr, request->http_uri);
            //uri文件位置变为服务器文件地址

            int fd_in = open(fileAddr, O_RDONLY);

            if(fd_in < 0) 
            {
                char resp[64] = "HTTP/1.1 404 Not Found\r\n\r\n";
                strcpy(buf, resp);
                error_write("Not Found");
            }

            else 
            {
                while (read(fd_in,buf,AcaBUF_SIZE) == AcaBUF_SIZE)
                {
                    AcaBUF_SIZE *= 2;
                    buf = (char *)realloc(buf, AcaBUF_SIZE + 64);
                }
                char* alterBuf = (char *)malloc(AcaBUF_SIZE+128);
                char* content_lenth[64];
                sprintf(content_lenth, "Content-Length: %ld\r\n", strlen(buf));
                strcpy(alterBuf, "HTTP/1.1 200 OK\r\n\r\nServer: liso_server\r\n");
                strcat(alterBuf, content_lenth);
                strcat(alterBuf, "Connection: keep-alive\r\n\r\n");
                strcat(alterBuf, buf);
                strcpy(buf, alterBuf);
                char requestLine[64];
                sprintf(requestLine, "%s %s %s", request->http_method, request->http_uri, request->http_version);
                log_write(requestLine, 0, 200);
            }
        }
    }//GET method

    else if (!strcmp(request->http_method, "POST"))
    {
        char requestLine[64];
        sprintf(requestLine, "%s %s %s", request->http_method, request->http_uri, request->http_version);
        log_write(requestLine, 0, 200);
    }

    free(request);
    //encapsulation  

    return buf;

}

int connection_handler(int server_sock, int client_sock)
{
    ssize_t readret = 0;
    char* buf = (char *)malloc(BUF_SIZE);
    AcaBUF_SIZE = BUF_SIZE;
    memset(buf, 0, AcaBUF_SIZE);
                                               
    if ((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
    {
        int pipRequestCnt = 0;
        int pipCapacity = 30; //default capacity for pipeline 
        char** BOB = (char **)malloc(pipCapacity * sizeof(char *));

        int requestReader, lastRequest = 0;
        for(requestReader = 0; requestReader < strlen(buf) - 3; ){
            if (buf[requestReader] == '\r' && buf[requestReader+2] == '\r'){
                    if (buf[requestReader+1] == '\n' && buf[requestReader+3] == '\n') //两层if条件判断能减少比较次数以提高服务器运行速度
                    //尽量减少比较
                    {
                        requestReader += 3; //减少重复比较. 不进行迭代器更新也可以.
                        BOB[pipRequestCnt] = (char *)malloc(requestReader - lastRequest + 10);
                        strncpy(BOB[pipRequestCnt], buf+lastRequest, requestReader - lastRequest + 1);
                        

                        char* handle_tempBuf = (char *)malloc(AcaBUF_SIZE);
                        strcpy(handle_tempBuf, request_handler(BOB[pipRequestCnt], readret, client_sock));

                        if (send(client_sock, handle_tempBuf, strlen(handle_tempBuf), 0) != strlen(handle_tempBuf))
                        {
                            close_socket(client_sock);
                            fprintf(stderr, "Error sending to client.\n");
                            //return EXIT_FAILURE;
                        }
                        pipRequestCnt ++;
                        lastRequest = requestReader+1;
                        free(handle_tempBuf);

                    }
            }
            
            else requestReader ++;

            if (pipRequestCnt == pipCapacity) 
            {
                pipCapacity *= 2;
                char** BOB = (char **)realloc(BOB, pipCapacity * sizeof(char *));
            }
        } // 流水线解析

        memset(buf, 0, AcaBUF_SIZE);
        free(BOB);
    }

    if (readret == -1) {
        fprintf(stderr, "Error reading from client socket.\n");
        if (close_socket(client_sock)){
            fprintf(stderr, "Error closing client socket.\n");
        }
        return 0;
    }

    return 1;
}

int main(int argc, char* argv[])
{
    int sock, client_sock;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;

    fprintf(stdout, "----- Liso Server -----\n");
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(sock, 5))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    fd_set sockets_current, sockets_ready; //当前socket序列与就绪序列
    FD_ZERO(&sockets_current);
    FD_SET(sock, &sockets_current); //服务端的socket加入集合
 
    /* finally, loop waiting for input and then write it back */
    while (1) {
        sockets_ready = sockets_current;
        if(select(FD_SETSIZE, &sockets_ready, NULL, NULL, NULL) < 0) {
            fprintf(stderr, "Select error.\n");
            return EXIT_FAILURE;
        }

        cli_size = sizeof(cli_addr);

        for (int i = 0; i < FD_SETSIZE; i ++) {
            if (FD_ISSET(i, &sockets_ready)) {
                if (i == sock) { //当前服务器
                    if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr, &cli_size)) == -1) {
                        close(sock);
                        fprintf(stderr, "Error accepting connection.\n");
                    }
                    FD_SET(client_sock, &sockets_current);
                    fprintf(stderr, "  server_sock: %d , client_sock: %d.\n", sock,client_sock);
                }
                else { //当前客户端
                    int flag;
                    if ((flag = fcntl(i, F_GETFL, 0)) < 0) {
                        fprintf(stderr, "Error getting flag.\n");
                    }
                    if (fcntl(i, F_SETFL, flag | O_NONBLOCK) < 0) { //设置非阻塞模式
                        fprintf(stderr, "Error setting nonblock.\n");
                    }
                    if (connection_handler(sock, i)) {
                        if ((flag = fcntl(i, F_GETFL, 0)) < 0) {
                            fprintf(stderr, "Error getting flag.\n");
                        }
                        if (fcntl(i, F_SETFL, (flag & ~O_NONBLOCK)) < 0) { //文件描述符状态删除
                            fprintf(stderr, "Error deleting nonblock.\n");
                        }
                        FD_CLR(i, &sockets_current); //客户端连接关闭
                    }  
                }
            }
        }
    }
    close_socket(sock);
    return EXIT_SUCCESS;
}