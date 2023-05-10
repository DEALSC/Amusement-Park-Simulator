#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "fair.h"

// This program stops the Fair server
void main() {
	// WRITE ALL THE NECESSARY CODE
	int clientSocket;
	struct sockaddr_in serverAddress;
	int status, bytesRcv;
	unsigned int buffer[3];

	// Create a socket to establish a connection to the fair server
 	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0) {
        printf("*** Client ERROR: Could not open socket.\n");
        exit(-1);
    }

	// Setup address
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

	// Connect to server
	status = connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	if (status < 0) {
		printf("*** CLIENT ERROR: Could not connect.\n");
		exit(-1);
	}

	// Send command to server
	buffer[0] = SHUTDOWN;
	printf("CLIENT: Sending SHUTDOWN command to server.\n");

	status = send(clientSocket, buffer, sizeof(buffer), 0);
	if (status < 0) {
		printf("*** CLIENT ERROR: Could not send command.\n");
		exit(-1);
	}

    // Close the socket
    close(clientSocket);
}
