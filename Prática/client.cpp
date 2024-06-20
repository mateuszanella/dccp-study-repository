/*
 * "O código é a própria documentação. E ele tem que estar em inglês."
 * - Steve McConnell
 */

#include "base.h"

void establish_connection(int sockfd, struct sockaddr_in &serv_addr);
void interact_with_server(int sockfd, struct sockaddr_in &serv_addr);
void retransmit_message(int sockfd, struct sockaddr_in &serv_addr, std::string &message, char *buffer, int &cwnd, int &ssthresh, int &acks_received);

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
    int conn_attempts = 0;
    int max_attempts = 5;
    bool connected = false;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        error("ERROR setting timeout for socket");
        return;
    }

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
    int res = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
    if (res > 0)
    {
        buffer[res] = '\0';
    }
    else if (res < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            error("ERROR no DCCP Response received within set timeframe");
        }
        else
        {
            error("ERROR receiving data from server");
            return;
        }
    }

    /*
     * If the server sends a [DCCP Response] back,
     * the client sends an ACK and starts the data exchange.
     */
    if (strcmp(buffer, DCCP_RESP) == 0)
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 150000;

        int result = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
        if (result < 0)
        {
            error("ERROR setting timeout for socket");
        }

        std::cout << "Received: " << buffer << std::endl;

        const char *ack = DCCP_ACK;
        send_message(sockfd, serv_addr, ack);
        std::cout << "Sent: " << ack << std::endl;

        /*
         * Here, the client waits for the case that the server does not receive the ACK.
         * The server will retransmit the [DCCP Response] packet to the client, and the client
         * will retransmit the ACK back to the server.
         */

        auto [sucess, received_msg] = await_response(sockfd, serv_addr, ack, 10, [&]()
                                                     { send_message(sockfd, serv_addr, ack); });

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
    std::string packet_type;

    int acks_received = 0; // Sucessful ACKs received from the server.
    int cwnd = 5;          // Congestion Window Size. Defines how many messages can be sent without receiving an ACK.
    int ssthresh = 10;     // Defines the threshold for the slow start algorithm.

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000;

    int result = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    if (result < 0)
    {
        error("ERROR setting timeout for socket");
    }

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

        std::string message = DCCP_DATA;

        if (packet_type == "1")
        {
            std::string input;
            std::string message = DCCP_DATA;
            std::cout << "Enter the message to send: ";
            std::getline(std::cin, input);
            message += " " + input;

            send_message(sockfd, serv_addr, message.c_str());
            std::cout << "\nSent: " << message << std::endl;
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

            acks_received++;

            if (strcmp(buffer, DCCP_ACK) == 0)
            {
                if (cwnd < ssthresh)
                {
                    cwnd *= 2;
                }
                else
                {
                    if (acks_received >= cwnd)
                    {
                        cwnd++;
                        acks_received = 0;
                    }
                }
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
        else if (res < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "No response received from server within specified timewindow." << std::endl;
                retransmit_message(sockfd, serv_addr, message, buffer, cwnd, ssthresh, acks_received);
            }
            else
            {
                error("ERROR receiving data from server");
                break;
            }
        }
    }
}

/*
 * This function emulates a simplified version of the CCID 2 congestion control algorithm using a mix of slow start
 * and congestion avoidance phases, typical in TCP-like protocols, but adapted here for demonstration purposes.
 * This function attempts to retransmit a message multiple times, corresponding to the current size of the
 * congestion window (cwnd), handling ACKs to adjust cwnd dynamically.
 *
 * The congestion control behaves as follows:
 * - Slow Start: On receiving an ACK, the cwnd is doubled until it reaches the slow start threshold (ssthresh),
 *   promoting rapid throughput increase. This phase is designed to quickly find the available bandwidth capacity.
 * - Congestion Avoidance: Once ssthresh is reached, cwnd is increased more conservatively to avoid network congestion.
 *   Specifically, cwnd is increased by one Maximum Segment Size (MSS) for each full window of packets acknowledged,
 *   simulating a careful and measured increase to maintain network stability.
 *
 * If an ACK is not received (indicated by a timeout), the function decreases cwnd by half and also adjusts
 * ssthresh to half of the current cwnd, reflecting a typical response to perceived network congestion.
 *
 */
void retransmit_message(int sockfd, struct sockaddr_in &serv_addr, std::string &message, char *buffer, int &cwnd, int &ssthresh, int &acks_received)
{
    for (int i = 0; i < cwnd; ++i)
    {
        std::cout << "Attempting to retransmit message. Attempt: " << i + 1 << " of " << cwnd << std::endl;
        send_message(sockfd, serv_addr, message.c_str());
        std::cout << "Sent: " << message << std::endl;

        int res = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
        int recv_errno = errno;
        if (res > 0)
        {
            buffer[res] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            if (strcmp(buffer, DCCP_ACK) == 0)
            {
                /*
                 * Manage the congestion window (cwnd) based on ACK reception using the TCP-like congestion control phases:
                 * Slow Start and Congestion Avoidance.
                 */

                acks_received++;

                /*
                 * Slow Start Phase:
                 * In this phase, the congestion window doubles for each ACK received, promoting rapid increase in throughput.
                 * This exponential growth continues until the cwnd reaches the slow start threshold (ssthresh).
                 */
                if (cwnd < ssthresh)
                {
                    cwnd *= 2; // Exponential growth: double the cwnd for each ACK until reaching the ssthresh.
                    // Note: This assumes the network can support such a rapid increase without leading to congestion.
                }
                /*
                 * Congestion Avoidance Phase:
                 * Once the slow start threshold is reached, we switch to congestion avoidance,
                 * where the cwnd is increased more gradually to avoid causing network congestion.
                 * This phase aims to optimize network throughput while maintaining network stability.
                 */
                else
                {
                    /*
                     * Increase cwnd by 1 MSS only after a full window of packets has been acknowledged,
                     * simulating approximately one increase per round-trip time (RTT).
                     *
                     * MSS: Maximum Segment Size (maximum amount of data that can be sent in a single segment).
                     */
                    if (acks_received >= cwnd)
                    {
                        cwnd++;            // Additive increase: add one MSS to the cwnd.
                        acks_received = 0; // Reset the count of ACKs after increasing the cwnd.
                    }
                }

                if (strcmp(buffer, DCCP_RESET) == 0)
                {
                    std::cout << "[DCCP Reset] packet received" << std::endl
                              << ". Closing connection." << std::endl;
                    exit(0);
                }

                if (strcmp(buffer, DCCP_CLOSE) == 0)
                {
                    std::cout << "[DCCP Close] packet received" << std::endl
                              << ". Closing connection." << std::endl;
                    exit(0);
                }
                return;
            }
            else
            {
                if (recv_errno == EAGAIN || recv_errno == EWOULDBLOCK)
                {
                    std::cout << "Timeout reached, no response received." << std::endl;

                    /*
                     * If the client reaches the maximum number of retransmissions without receiving an ACK,
                     * it will decrease the congestion window and the slow start threshold.
                     */
                    if (i == cwnd - 1)
                    {
                        std::cout << "Decreasing congestion window due to repeated timeouts." << std::endl;
                        ssthresh = std::max(2, cwnd / 2); // Set the slow start threshold to half the current congestion windowß
                        cwnd = std::max(1, cwnd / 2);     // Halve the congestion window
                        return;
                    }
                }
                else
                {
                    error("ERROR receiving data from server");
                    return;
                }
            }
        }
    }
}
