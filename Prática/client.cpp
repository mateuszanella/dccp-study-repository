#include "base.h"

void interact_with_server(int sockfd, struct sockaddr_in &serv_addr)
{
    char buffer[BUFFER_SIZE];
    std::string user_input;

    // Connection establishment
    const char *conn_msg = "connect";
    send_message(sockfd, serv_addr, conn_msg);
    std::cout << "Connection request sent to server" << std::endl;

    // Await ACK from server
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
    if (n > 0)
    {
        buffer[n] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        if (std::string(buffer).find("ACK: connect") != std::string::npos)
        {
            std::cout << "Connection established successfully." << std::endl;
        }
        else
        {
            std::cout << "Failed to establish connection." << std::endl;
            return;
        }
    }

    // Constant stream of data
    while (true)
    {
        std::cout << "Enter message (type 'terminate' to end): ";
        std::getline(std::cin, user_input);

        send_message(sockfd, serv_addr, user_input.c_str());
        std::cout << "Sent: " << user_input << std::endl;

        // Await ACK from server
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
        if (n > 0)
        {
            buffer[n] = '\0';
            std::cout << "Received: " << buffer << std::endl;
        }

        if (user_input == "terminate")
        {
            std::cout << "Termination message sent to server. Closing connection." << std::endl;
            break;
        }
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    std::string server_ip;

    std::cout << "Enter server IP address: ";
    std::cin >> server_ip;
    std::cin.ignore(); // Clear newline

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        error("ERROR opening socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        error("ERROR converting server IP address");
    }

    interact_with_server(sockfd, serv_addr);

    close(sockfd);
    return 0;
}
