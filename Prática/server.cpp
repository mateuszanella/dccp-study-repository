#include "base.h"
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>

#define PORT 8080
#define BUFFER_SIZE 1024

std::mutex log_mutex;
std::vector<std::thread> threads;
std::mutex threads_mutex;
std::condition_variable cv;
bool done = false;

void log(const std::string &message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << message << std::endl;
}

void handle_client(int sockfd, struct sockaddr_in cli_addr) {
    char buffer[BUFFER_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    // Send ACK for connection establishment
    std::string ack = "ACK: connect";
    sendto(sockfd, ack.c_str(), ack.length(), 0, (struct sockaddr *)&cli_addr, cli_len);
    log("Sent ACK to IP: " + std::string(inet_ntoa(cli_addr.sin_addr)) +
        ", Port: " + std::to_string(ntohs(cli_addr.sin_port)));

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n <= 0)
            break;

        buffer[n] = '\0'; // Null-terminate the received string
        log("Received: " + std::string(buffer));

        std::string ack = "ACK: " + std::string(buffer);
        sendto(sockfd, ack.c_str(), ack.length(), 0, (struct sockaddr *)&cli_addr, cli_len);
        log("Sent ACK to IP: " + std::string(inet_ntoa(cli_addr.sin_addr)) +
            ", Port: " + std::to_string(ntohs(cli_addr.sin_port)));

        if (strcmp(buffer, "terminate") == 0) {
            log("Termination signal received from IP: " + std::string(inet_ntoa(cli_addr.sin_addr)) +
                ", Port: " + std::to_string(ntohs(cli_addr.sin_port)) + ". Closing connection.");
            break;
        }
    }
}

void cleanup_threads() {
    while (!done) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(threads_mutex);
        for (auto it = threads.begin(); it != threads.end();) {
            if (it->joinable()) {
                it->join();
                it = threads.erase(it);
            } else {
                ++it;
            }
        }
    }
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

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

    log("Server is running on port " + std::to_string(PORT) + " and waiting for connections...");

    std::thread cleaner_thread(cleanup_threads);

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n > 0) {
            buffer[n] = '\0';
            log("Connection established with client at IP: " + std::string(inet_ntoa(cli_addr.sin_addr)) +
                ", Port: " + std::to_string(ntohs(cli_addr.sin_port)));

            std::lock_guard<std::mutex> lock(threads_mutex);
            threads.emplace_back(std::bind(handle_client, sockfd, cli_addr));
        }
    }

    done = true;
    cv.notify_all();
    cleaner_thread.join();

    close(sockfd);
    return 0;
}
