#include "base.h"
#include <thread>
#include <vector>
#include <future>
#include <chrono>

void establish_connection(std::vector<std::thread> &threads, int sockfd, struct sockaddr_in cli_addr, char *buffer);
void handle_client(int sockfd, struct sockaddr_in cli_addr);

/*
 * The server is set up in a way that it can handle multiple clients at the same time.
 * When a client sends a [DCCP Request], the server will start a new thread to handle the
 * connection with that client. The server will then send a [DCCP Response] to the client
 * and wait for an ACK. If the ACK is received, the connection is established and the server
 * will start a new thread to handle the client's messages.
 * If no ACK is received, the server will close the connection.
 */
int main()
{
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    std::vector<std::thread> threads;

    /*
     * Setting up socket for communication with the clients.
     */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        error("ERROR opening socket");
    }

    /*
     * Setting up server address.
     */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding socket");
    }

    std::cout << "Server is running on port " << PORT << " and waiting for connections..." << std::endl;

    /*
     * The server will keep listening for incoming messages from clients.
     */
    while (true)
    {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n > 0 && strcmp(buffer, DCCP_REQ) == 0)
        {
            buffer[n] = '\0';
            std::cout << "Received: " << buffer << " from IP: "
                      << inet_ntoa(cli_addr.sin_addr)
                      << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

            /*
             * Start a new thread to handle the connection with the client.
             */
            threads.push_back(std::thread(establish_connection, std::ref(threads), sockfd, cli_addr, buffer));
        }
    }

    /*
     * Wait for all threads to finish before closing the server.
     * Code should't really hit this point, as nothing breaks the loop.
     * Good practice to have it here anyway.
     */
    for (auto &t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    close(sockfd);
    return 0;
}

void establish_connection(std::vector<std::thread> &threads, int sockfd, struct sockaddr_in cli_addr, char *buffer)
{
    /*
     * This functions runs with a timeout, and is terminated if no ACK is received.
     *
     * The server begins by sending a [DCCP Response] to the client and wait for an ACK.
     */
    auto connection_task = [&]()
    {
        std::string msg = DCCP_RESP;
        send_message(sockfd, cli_addr, msg.c_str());
        std::cout << "Sent: " << msg << " to IP: " << inet_ntoa(cli_addr.sin_addr)
                  << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

        socklen_t cli_len = sizeof(cli_addr);

        /*
         * Awaits for the ACK.
         */
        int rec = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);

        if (rec > 0 && strcmp(buffer, DCCP_ACK) == 0)
        {
            std::cout << "Connection established with IP: " << inet_ntoa(cli_addr.sin_addr)
                      << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

            threads.push_back(std::thread(handle_client, sockfd, cli_addr));
        }
    };

    /*
     * Timeout for the connection attempt.
     */
    std::future<void> future = std::async(std::launch::async, connection_task);
    if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
        std::cout << "Connection attempt timed out." << std::endl;
        return;
    }
    // else
    // {
    //     future.get();
    // }
}

void handle_client(int sockfd, struct sockaddr_in cli_addr)
{
    char buffer[BUFFER_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    while (true)
    {
        /*
         * After the connection is established, the server will enter a listening stage.
         */
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n <= 0)
            break;

        buffer[n] = '\0';
        std::cout << "Received: " << buffer << std::endl;

        /*
         * If the client sends a [DCCP Reset], the server will close the connection.
         * No ACK is sent back to the client, since the connection is being forcefully closed.
         */
        if (strcmp(buffer, DCCP_RESET) == 0)
        {
            std::cout << "[DCCP Reset] packet received from IP: " << inet_ntoa(cli_addr.sin_addr)
                      << ", Port: " << ntohs(cli_addr.sin_port) << ". Closing connection." << std::endl;
            break;
        }

        std::string ack = "ACK: " + std::string(buffer);
        send_message(sockfd, cli_addr, ack.c_str());
        std::cout << "Sent: " << ack << " to IP: " << inet_ntoa(cli_addr.sin_addr)
                  << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;
    }
}
