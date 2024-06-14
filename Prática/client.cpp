/*
 * "O código é a própria documentação. E ele tem que estar em inglês."
 * - Steve McConnell
 */

#include "base.h"

void establish_connection(int sockfd, struct sockaddr_in &serv_addr);
void interact_with_server(int sockfd, struct sockaddr_in &serv_addr);

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    std::string server_ip;

    std::cout << "Enter server IP address (leave empty for localhost): ";
    std::getline(std::cin, server_ip);
    if (server_ip.empty())
    {
        server_ip = "127.0.0.1";
    }

    /*
     * Setting up socket for communication with the server.
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
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        error("ERROR converting server IP address");
    }

    /*
     * Start the interaction with the server.
     */
    establish_connection(sockfd, serv_addr);

    close(sockfd);
    return 0;
}

void establish_connection(int sockfd, struct sockaddr_in &serv_addr)
{
    char buffer[BUFFER_SIZE];
    std::string packet_type, message;

    /*
     * Starts the three way handshake with the server.
     * The client sends a [DCCP Request] to the server.
     */
    const char *conn_msg = DCCP_REQ;
    send_message(sockfd, serv_addr, conn_msg);
    std::cout << "Sent: " << conn_msg << std::endl;

    /*
     * The client waits for a [DCCP Response] from the server.
     */
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);

    /*
     * If the server sends a [DCCP Response] back,
     * the client sends an ACK and starts the data exchange.
     */
    if (n > 0)
    {
        buffer[n] = '\0';
    }
    else
    {
        std::cout << "Connection attempt timed out. Closing connection.\n\n";
        return;
    }

    if (strcmp(buffer, DCCP_RESP) == 0)
    {
        std::cout << "Received: " << buffer << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        const char *ack = DCCP_ACK;
        send_message(sockfd, serv_addr, ack);
        std::cout << "Sent: " << ack << std::endl;

        interact_with_server(sockfd, serv_addr);
    }

    /*
     * If the server does not respond with a [DCCP Response] within the expected
     * time window, the client closes the connection. The DCCP protocol allows for
     * a policy of retransmissions to be implemented. But for the sake of simplicity,
     * we will not implement it here.
     */
    else
    {
        std::cout << "Connection attempt failed. Closing connection.\n\n";
        return;
    }
}

void interact_with_server(int sockfd, struct sockaddr_in &serv_addr)
{
    char buffer[BUFFER_SIZE];
    socklen_t serv_len = sizeof(serv_addr);
    std::string packet_type, message;

    /*
     * The client can now start sending messages to the server.
     *
     * The client will send a message and may an ACK from the server.
     * The DCCP protocol is considered non-reliable, given that he does not guarantee
     * the delivery of messages, neither the order in which they are received.
     * (They do contain a checksum and a sequence number, to help with the flow control)
     *
     * By default, DCCP does not implement retransmissions, but it is possible to
     * implement a policy of retransmissions in the application layer.
     *
     * Now you may ask: "Why have an ACK if there won't be a retransmission?"
     * The ACK is used by the innate congestion protocol of DCCP to control the flow
     * of messages between the client and the server. In the case of the CCID 2
     * (wich is the one implemented here), the client keeps count for sent messages
     * and ACKs received. It uses this to determine the packet loss, and adjust the
     * congestion window accordingly.
     */
    while (true)
    {
        std::cout << "\nSelect a packet type to send: \n";
        std::cout << "1. [DCCP Data]\n2. [DCCP CloseReq]\n3. [DCCP Reset]\n";
        std::cout << "Enter your choice: ";
        std::getline(std::cin, packet_type);

        if (packet_type == "1")
        {
            std::string input;
            std::string message = DCCP_DATA;
            std::cout << "Enter the message to send: ";
            std::getline(std::cin, input);
            message += " " + input;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            send_message(sockfd, serv_addr, input.c_str());
            std::cout << "\nSent: " << input << std::endl;
        }
        else if (packet_type == "2")
        {
            const char *close_req = DCCP_CLOSE_REQ;
            send_message(sockfd, serv_addr, close_req);
            std::cout << "\nSent: " << close_req << std::endl;
        }
        else if (packet_type == "3")
        {
            const char *reset = DCCP_RESET;
            send_message(sockfd, serv_addr, reset);
            std::cout << "\nSent: " << reset << std::endl;
            std::cout << "Terminating connection.\n\n";
            break;
        }
        else
        {
            std::cout << "\nInvalid packet type. Please try again." << std::endl;
            continue;
        }

        /*
         * After sending a message, the client waits for a response from the server.
         */
        int res = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
        if (res > 0)
        {
            buffer[res] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            if (strcmp(buffer, DCCP_ACK) == 0)
            {
                continue;
            }

            /*
             * In the case that the server does not receive a [DCCP Ack] packet, but the client
             * has entered a sending loop, the server will retransmit a [DCCP Response] packet to the client,
             * in the hopes of reestablishing the connection.
             *
             * In this case, the client will send an ACK back to the server and continue the interaction.
             * A policy to retransmit the previous lost [DCCP Data] packet could be implemented.
             */
            if (strcmp(buffer, DCCP_RESP) == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                const char *ack = DCCP_ACK;
                send_message(sockfd, serv_addr, ack);
                std::cout << "Sent: " << ack << std::endl;

                continue;
            }

            /*
             * If the server sends a [DCCP Reset], the client will close the connection.
             * No ACK is sent back to the client, since the connection is being forcefully closed.
             */
            if (strcmp(buffer, DCCP_RESET) == 0)
            {
                std::cout << "[DCCP Reset] packet received" << std::endl
                          << ". Closing connection." << std::endl;
                break;
            }

            /*
             * If the server sends a [DCCP CloseReq], the client will send a
             * [DCCP Close] back to the server and close the connection.
             */
            if (strcmp(buffer, DCCP_CLOSE) == 0)
            {
                std::cout << "[DCCP Close] packet received" << std::endl
                          << ". Closing connection." << std::endl;
                break;
            }
        }

        /*
         * Reset variables for next iteration.
         */
        packet_type.clear();
        message.clear();
        buffer[0] = '\0';

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}
