/*
aesdsocket.c - A simple TCP socket server that listens on a specified port and echoes back any data received from clients
Author: Muthuu SVS
Date: 02-12-2026
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <syslog.h>
#include <signal.h>

#define PORT 9000
#define BUFFER_SIZE 1024

//string to store data received from client
const char *outputfile = "/var/tmp/aesdsocketdata";

//signal handler for graceful shutdown
void signal_handler(int signum)
{
    syslog(LOG_INFO, "Caught signal %d, exiting", signum);
    exit(0);
}

int main(int argc, char *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    //signal handling for graceful shutdown
    if(sigaction(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("sigaction failed");
        return -1;
    }
    if(sigaction(SIGTERM, signal_handler) == SIG_ERR)
    {
        perror("sigaction failed");
        return -1;
    }

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("socket failed");
        return -1;
    }

    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        perror("bind failed");
        close(server_fd);
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) 
    {
        perror("listen failed");
        close(server_fd);
        return -1;
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
        {
            perror("accept failed");
            close(server_fd);
            return -1;
        }

        syslog(LOG_INFO, "Accepted connection from %s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        //receive data from the client
        int valread = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (valread < 0)
        {
            perror("read failed");
            close(new_socket);
            continue;
        }
        //echo data back to client
        send(new_socket, buffer, valread, 0);
        syslog(LOG_INFO, "Echoed back data to %s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // Close the connection
        close(new_socket);
        syslog(LOG_INFO, "Closed connection from %s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    }

    return 0;
}