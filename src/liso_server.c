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
#include "parse.h"
#include "log.h"

#define ECHO_PORT 9999
#define BUF_SIZE 998192

extern Request* parse(char *buffer, int size,int socketFd);

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

char* request_handler(char* buffer, int readret, int client_sock){
    Request *request = (Request *) malloc(sizeof(Request));
    char* buf = (char *)malloc(BUF_SIZE);
    strcpy(buf, buffer);
    request = parse(buf, BUF_SIZE, client_sock);
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
            strcpy(alterBuf, "HTTP/1.1 200 OK\r\n");
            strcat(alterBuf, buf);
            strcpy(buf, alterBuf);
            char requestLine[64];
            sprintf(requestLine, "%s %s %s", request->http_method, request->http_uri, request->http_version);
            log_write(requestLine, 0, 200);
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

int main(int argc, char* argv[])
{
    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char* buf = (char *)malloc(BUF_SIZE);

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

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                    &cli_size)) == -1)
        {
            close(sock);
            fprintf(stderr, "Error accepting connection.\n");
            return EXIT_FAILURE;
        }

        readret = 0;
        AcaBUF_SIZE = BUF_SIZE;

        while((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
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
                            pipRequestCnt ++;
                            lastRequest = requestReader+1;
                        }
                }
                
                else requestReader ++;

                if (pipRequestCnt == pipCapacity) 
                {
                    pipCapacity *= 2;
                    char** BOB = (char **)realloc(BOB, pipCapacity * sizeof(char *));
                }
            } // 流水线解析

            int request_handle_iter;
            char* handle_tempBuf = (char *)malloc(AcaBUF_SIZE);
            for (request_handle_iter = 0; request_handle_iter < pipRequestCnt; request_handle_iter ++)
            {
                if (request_handle_iter == 0)
                     strcpy(handle_tempBuf, request_handler(BOB[request_handle_iter], readret, client_sock));
                else strcat(handle_tempBuf, request_handler(BOB[request_handle_iter], readret, client_sock));
                if (AcaBUF_SIZE - strlen(handle_tempBuf) < 64)
                {
                    AcaBUF_SIZE *= 2;
                    handle_tempBuf = (char *)realloc(handle_tempBuf, AcaBUF_SIZE + 64);
                }
            }

            if (send(client_sock, handle_tempBuf, strlen(handle_tempBuf), 0) != strlen(handle_tempBuf))
            {
                close_socket(client_sock);
                close_socket(sock);
                fprintf(stderr, "Error sending to client.\n");
                return EXIT_FAILURE;
            }

            memset(buf, 0, AcaBUF_SIZE);
            free(BOB);
        } 

        if (readret == -1)
        {
            close_socket(client_sock);
            close_socket(sock);
            fprintf(stderr, "Error reading from client socket.\n");
            return EXIT_FAILURE;
        }

        if (close_socket(client_sock))
        {
            close_socket(sock);
            fprintf(stderr, "Error closing client socket.\n");
            return EXIT_FAILURE;
        }
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
