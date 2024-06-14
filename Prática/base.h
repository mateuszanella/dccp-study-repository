/*
 * Defines base functions, constants and includes for the server and client.
 */

#ifndef BASE_H
#define BASE_H

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

#define DCCP_REQ "[DCCP Request]"
#define DCCP_RESP "[DCCP Response]"
#define DCCP_ACK "[DCCP Ack]"
#define DCCP_DATA "[DCCP Data]"
#define DCCP_CLOSE_REQ "[DCCP CloseReq]"
#define DCCP_CLOSE "[DCCP Close]"
#define DCCP_RESET "[DCCP Reset]"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void send_message(int sockfd, struct sockaddr_in &addr, const char *message)
{
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr));
}

#endif
