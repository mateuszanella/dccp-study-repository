#include "base.h"
#include <vector>
#include <future>

void establish_connection(int sockfd, struct sockaddr_in cli_addr);
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
        if (n > 0)
        {
            buffer[n] = '\0';

            if (strcmp(buffer, DCCP_REQ) == 0)
            {
                std::cout << "Received: " << buffer << " from IP: "
                          << inet_ntoa(cli_addr.sin_addr)
                          << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

                /*
                 * Start a new thread to handle the connection with the client.
                 */
                threads.push_back(std::thread(establish_connection, sockfd, cli_addr));
            }
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

void establish_connection(int sockfd, struct sockaddr_in cli_addr)
{
    socklen_t cli_len = sizeof(cli_addr);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;

    int result = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    if (result < 0)
    {
        error("ERROR setting timeout for socket");
    }

    /*
     * This functions runs with a set amount of max tries, and is terminated if no ACK is received.
     *
     * The server begins by sending a [DCCP Response] to the client and wait for an ACK.
     */
    std::string response_msg = DCCP_RESP;
    send_message(sockfd, cli_addr, response_msg.c_str());
    std::cout << "Sent: " << response_msg << " to IP: " << inet_ntoa(cli_addr.sin_addr)
              << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

    /*
     * Awaits for the ACK.
     */
    auto [success, received_msg] = await_response(sockfd, cli_addr, response_msg.c_str(), 5, [&]()
                                                  {
        send_message(sockfd, cli_addr, response_msg.c_str());
        std::cout << "Resending: " << response_msg << " to IP: "
                << inet_ntoa(cli_addr.sin_addr)
                << ", Port: " << ntohs(cli_addr.sin_port) << std::endl; });

    if (!success)
    {
        std::cout << "Failed to establish connection after " << 5 << " attempts." << std::endl;
        return;
    }

    if (received_msg == DCCP_ACK)
    {
        std::cout << "Received " << received_msg << std::endl
                  << "Connection established with IP: "
                  << inet_ntoa(cli_addr.sin_addr)
                  << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;

        handle_client(sockfd, cli_addr);
    }
    else
    {
        std::cout << "No ACK received. Closing connection with IP: "
                  << inet_ntoa(cli_addr.sin_addr)
                  << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;
        return;
    }
}

void handle_client(int sockfd, struct sockaddr_in cli_addr)
{
    char buffer[BUFFER_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    while (true)
    {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        /*
         * After the connection is established, the server will enter a listening stage.
         */
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n > 0)
        {
            buffer[n] = '\0';

            if (strcmp(buffer, DCCP_ACK) == 0)
            {
                continue;
            }

            /*
             * If the client sends a [DCCP Reset], the server will close the connection.
             * No ACK is sent back to the client, since the connection is being forcefully closed.
             */
            if (strcmp(buffer, DCCP_RESET) == 0)
            {
                std::cout << "[DCCP Reset] packet received from IP: "
                          << inet_ntoa(cli_addr.sin_addr)
                          << ", Port: " << ntohs(cli_addr.sin_port)
                          << ". Closing connection." << std::endl;
                return;
            }

            /*
             * If the client sends a [DCCP CloseReq], the server will send a
             * [DCCP Close] back to the client and close the connection.
             */
            if (strcmp(buffer, DCCP_CLOSE_REQ) == 0)
            {
                std::cout << "[DCCP CloseReq] packet received from IP: "
                          << inet_ntoa(cli_addr.sin_addr)
                          << ", Port: " << ntohs(cli_addr.sin_port)
                          << ". Closing connection." << std::endl;

                std::string close = DCCP_CLOSE;
                send_message(sockfd, cli_addr, close.c_str());
                std::cout << "Sent: " << close << " to IP: " << inet_ntoa(cli_addr.sin_addr)
                          << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;
                return;
            }

            /*
             * If the client sends a [DCCP Data], the server will send a [DCCP Ack] back to the client.
             *
             * Both server and clients may wish to send data along with the acknowledgment.
             * For that, the server will send a [DCCP DataAck] back to the client.
             * For more info, check the 4. Packets roadmap section.
             */
            std::string ack = DCCP_ACK;
            send_message(sockfd, cli_addr, ack.c_str());
            std::cout << "Sent: " << ack << " to IP: " << inet_ntoa(cli_addr.sin_addr)
                      << ", Port: " << ntohs(cli_addr.sin_port) << std::endl;
        }
        else
        {
            // std::cout << "No data received on cycle. Waiting for new packets...\n";
        }
    }
}
