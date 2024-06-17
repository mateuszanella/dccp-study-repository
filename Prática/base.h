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
#include <utility>
#include <chrono>
#include <thread>
#include <functional>

#define PORT 8080
#define BUFFER_SIZE 4096
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

std::pair<bool, std::string> await_response(int sockfd, struct sockaddr_in cli_addr, const char* response_msg, int max_attempts, std::function<void()> on_response)
{
    char buffer[BUFFER_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    int conn_attempts = 0;
    bool connected = false;
    std::string received_msg;

    while (conn_attempts < max_attempts && !connected)
    {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);

        if (n > 0)
        {
            buffer[n] = '\0';
            received_msg = std::string(buffer);
            connected = true;
        }
        else if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                send_message(sockfd, cli_addr, response_msg);
                std::cout << "Attempt " << conn_attempts + 1 << ": Resending: " << response_msg << " to IP: "
                          << inet_ntoa(cli_addr.sin_addr)
                          << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;
            }
            else
            {
                return {false, "ERROR receiving data from server"};
            }
        }

        conn_attempts++;
    }

    if (!connected)
    {
        return {false, "Failed to receive message after " + std::to_string(max_attempts) + " attempts."};
    }
    else
    {
        if(on_response) on_response();
    }

    return {true, received_msg};
}

#endif
