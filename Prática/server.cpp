#include "base.h"
#include <thread>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int sockfd, struct sockaddr_in cli_addr) {
    char buffer[BUFFER_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    // Send ACK for connection establishment
    std::string ack = "ACK: connect";
    send_message(sockfd, cli_addr, ack.c_str());
    std::cout << "Sent ACK to IP: " << inet_ntoa(cli_addr.sin_addr)
              << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n <= 0)
            break;

        buffer[n] = '\0'; // Null-terminate the received string
        std::cout << "Received: " << buffer << std::endl;

        std::string ack = "ACK: " + std::string(buffer);
        send_message(sockfd, cli_addr, ack.c_str());
        std::cout << "Sent ACK to IP: " << inet_ntoa(cli_addr.sin_addr)
                  << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

        if (strcmp(buffer, "terminate") == 0) {
            std::cout << "Termination signal received from IP: " << inet_ntoa(cli_addr.sin_addr)
                      << ", Port: " << ntohs(cli_addr.sin_port) << ". Closing connection." << std::endl;
            break;
        }
    }
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    std::vector<std::thread> threads;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("Error opening socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("Error on binding socket");
    }

    std::cout << "Server is running on port " << PORT << " and waiting for connections..." << std::endl;

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "Connection established with client at IP: "
                      << inet_ntoa(cli_addr.sin_addr)
                      << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

            // Create a new thread to handle the client
            threads.push_back(std::thread(handle_client, sockfd, cli_addr));
        }
    }

    // Join threads before exiting
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    close(sockfd);
    return 0;
}
