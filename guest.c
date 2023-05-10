#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "fair.h"

void handleSig1() {
	printf("Guest %u has entered the ride.\n", getpid());
}

void handleSig2(){
	printf("Guest %u has exited the ride.\n", getpid());
}
// This program takes 3 command line arguments.
// 1 - The number of tickets that the guest has (e.g., 5 to 40)
// 2 - The maximum time (in seconds) that the guest is willing to wait in line for a ride (e.g., 600 - 1200)
// 3 - The first ride that this guest wants to go on (i.e., 0 to NUM_RIDES)

void main(int argc, char *argv[]) {
	// Set the random seed
	srand(time(NULL));

	// Get the number of tickets, willing wait time and first ride from the command line arguments
	unsigned int ticket = atoi(argv[1]);
	unsigned int wait_time = atoi(argv[2]);
	unsigned int ride_to_ride = atoi(argv[3]);
	unsigned int guestId = getpid();
	int request;

	int clientSocket;
	struct sockaddr_in serverAddress;
	int status, bytesRcv;
	unsigned int buffer[3], response[3];

	//Only used in admission
	Ride rides[NUM_RIDES];
	unsigned char recvBuffer[sizeof(buffer) + sizeof(rides)*NUM_RIDES];

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

	// Now simulate the going on rides until no more tickets remain (you will want to change the "1" here)
	unsigned char admission = 1, contCheck = 1;
	while(ticket>0) {

		// Connect to server
		if(contCheck){
			status = connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
			if (status < 0) {
				printf("*** CLIENT ERROR: Could not connect.\n");
				exit(-1);
			}
			contCheck = 0;
		}
		// Request a admission to the fair.  If cannot get in (i.e., MAX_GUESTS reached), then quit.
		if(admission){
			response[0] = ADMIT;
			status = send(clientSocket, response, sizeof(response), 0);
			if (status < 0) {
				printf("*** CLIENT ERROR: Could not send command.\n");
				exit(-1);
			}

			recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			memcpy(buffer, recvBuffer, sizeof(buffer));
			if(buffer[0] == NO){
				break;
			}
			memcpy(rides, recvBuffer + sizeof(buffer), sizeof(rides));
			admission = 0;
		}

		// Make sure that the guest has enough tickets for the desired ride
		// otherwise chose a different ride
		while(1){
			if(ticket >= rides[ride_to_ride].ticketsRequired){
				break;
			}
			ride_to_ride = rand() % 10;
		}

		// Get wait time estimate for that ride
		response[0] = GET_WAIT_ESTIMATE;
		response[1] = ride_to_ride;
		status = send(clientSocket, response, sizeof(response), 0);
		if (status < 0) {
			printf("*** CLIENT ERROR: Could not send command.\n");
			exit(-1);
		}
		recv(clientSocket, buffer, sizeof(buffer), 0);
		if(buffer[0] > wait_time){
			contCheck = 0;
			continue;
		}

		// If the guest is willing to wait, then get into line for that ride
		response[0] = GET_IN_LINE;
		response[1] = ride_to_ride;
		response[2] = guestId;
		status = send(clientSocket, response, sizeof(response), 0);
		if (status < 0) {
			printf("*** CLIENT ERROR: Could not send command.\n");
			exit(-1);
		}
		recv(clientSocket, buffer, sizeof(buffer), 0);
		if(buffer[0] == NO){
			contCheck = 0;
			continue;
		}
		ticket -= buffer[1];

		// Wait until the ride has boarded this guest, completed the ride and unboarded the guest
		sigset_t setUsr1, setUsr2;
		sigemptyset(&setUsr1);
		sigaddset(&setUsr1, SIGUSR1);
		sigemptyset(&setUsr2);
		sigaddset(&setUsr2, SIGUSR2);
		int signal_number_usr1, signal_number_usr2;

		signal(SIGUSR1, handleSig1);
		sigwait(&setUsr1, &signal_number_usr1);

		signal(SIGUSR2, handleSig2);
		sigwait(&setUsr2, &signal_number_usr2);

		// Delay a bit (DO NOT CHANGE THIS LINE)
		usleep(100000);

		// Choose a new ride at random
		ride_to_ride = rand() % 10;

		close(clientSocket);
		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket < 0) {
				printf("*** Client ERROR: Could not open socket.\n");
				exit(-1);
		}
		contCheck = 1;
	}

	status = connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	if (status < 0) {
		printf("*** CLIENT ERROR: Could not connect.\n");
		exit(-1);
	}

	// When out of tickets, inform the Fair that you are leaving
	response[0] = LEAVE_FAIR;
	status = send(clientSocket, response, sizeof(response), 0);
	if (status < 0) {
		printf("*** CLIENT ERROR: Could not send command.\n");
		exit(-1);
	}
	printf("Guest %u is leaving the fair.\n", guestId);

	// Close the socket
  	close(clientSocket);
	kill(guestId, SIGTERM);
}
