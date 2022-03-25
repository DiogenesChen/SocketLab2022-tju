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

#define ECHO_PORT 9999
#define BUF_SIZE 8192

extern Request* parse(char *buffer, int size,int socketFd);

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char* buf = (char *)malloc(BUF_SIZE);
    int AcaBUF_SIZE = BUF_SIZE;

    fprintf(stdout, "----- Echo Server -----\n");
    
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

        while((readret = recv(client_sock, buf, AcaBUF_SIZE, 0)) >= 1)
        {
            Request *request = (Request *) malloc(sizeof(Request));
            request = parse(buf, BUF_SIZE, client_sock);
            if (request == NULL || readret == BUF_SIZE)
            {
                char resp[64] = "HTTP/1.1 400 Bad request\r\n\r\n";
                strcpy(buf, resp);
            }//Bad request

            else if (strcmp(request->http_method, "GET") && strcmp(request->http_method, "HEAD") && strcmp(request->http_method, "POST"))
            {
                char resp[64] = "HTTP/1.1 501 Not Implemented\r\n\r\n";
                strcpy(buf, resp);

            }//not implemented

            else if (strcmp(request->http_version, "HTTP/1.1"))
            {
                char resp[64] = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
                strcpy(buf, resp);

            }//Version not supported

            else if (!strcmp(request->http_method, "HEAD"))
            {
                char resp[64] = "HTTP/1.1 200 OK\r\n";
                strcpy(buf, resp);

            }//Head method

            else if (!strcmp(request->http_method, "GET"))
            {
                char fileAddr[64] = "/home/project-1/static_site";

                if (!strcmp(request->http_uri, "/")) strcat(fileAddr, "/index.html");
                else strcat(fileAddr, request->http_uri);
                //uri文件位置变为服务器文件地址

                int fd_in = open(fileAddr, O_RDONLY);

                if(fd_in < 0) 
                {
                    char resp[64] = "HTTP/1.1 404 Not Found\r\n\r\n";
                    strcpy(buf, resp);
                }

                else 
                {
                    while (read(fd_in,buf,AcaBUF_SIZE) == AcaBUF_SIZE)
                    {
                        AcaBUF_SIZE *= 2;
                        buf = (char *)realloc(buf, AcaBUF_SIZE);
                    }

                    char* alterBuf = (char *)malloc(AcaBUF_SIZE+64);
                    strcpy(alterBuf, "HTTP/1.1 200 OK\r\n");
                    strcat(alterBuf, buf);
                    buf = (char *)realloc(buf, AcaBUF_SIZE+64);
                    strcpy(buf, alterBuf);
                }
            }//GET method

            free(request);
            //encapsulation     

            if (send(client_sock, buf, strlen(buf), 0) != strlen(buf))
            {
                close_socket(client_sock);
                close_socket(sock);
                fprintf(stderr, "Error sending to client.\n");
                return EXIT_FAILURE;
            }
            memset(buf, 0, AcaBUF_SIZE);
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
