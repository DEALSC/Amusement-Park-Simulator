#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>

// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should wait for a client to send a request, process it, and then
// close the client connection and wait for another client.  The requests that may be
// handled are as follows:
//
//   SHUTDOWN - causes the fair server to go offline.  No response is returned.
//
//   ADMIT - contains guest's process ID as well. return a list of all rides and their
//			 ticketRequirements.
//
//	 GET_WAIT_ESTIMATE - takes a ride ID as well.   It then returns an estimate as to
//						 how long of a wait (in seconds) the guest would have to wait
//						 in order to get on the ride.
//
//	 GET_IN_LINE - takes a ride ID and guest's process ID as well.  It then causes the
//				   guest to get in line for the specified ride ... assuming that the
//				   ride ID was valid and that the line hasn't reached its maximum.
//				   An OK response should be returned if all went well, otherwise NO.
//
//   LEAVE_FAIR - takes a guest's process ID.  It then causes the guest to leave the fair.
//				  No response is returned.

void *handleIncomingRequests(void *x) {
	Fair *f = (Fair *)x;

	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddr;
	int status, addrSize;
	unsigned int buffer[3], response[3];
	//buffer[0] = guest request
	//buffer[1] = ride ID

	//response[0] = YES/NO
	//response[1] = guest ID

	// Create the server socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket < 0) {
		printf("*** SERVER ERROR: Could not open socket.\n");
		exit(-1);
	}

	// Setup the server address
	memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

	// Bind the server socket
	status = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (status < 0) {
		printf("*** SERVER ERROR: Could not bind socket.\n");
		exit(-1);
	}

	// Set up the line-up to handle up to 5 clients in line
	status = listen(serverSocket, 5);
	if (status < 0) {
		printf("*** SERVER ERROR: Could not listen on socket.\n");
		exit(-1);
	}

	// Wait for clients now
	while(1){
		addrSize = sizeof(clientAddr);
		clientSocket = accept(serverSocket,(struct sockaddr *)&clientAddr, &addrSize);
		if (clientSocket < 0) {
			printf("*** SERVER ERROR: Could accept incoming client connection.\n");
			exit(-1);
		}
		printf("SERVER: Received client connection.\n");

		while(1){
			// Get the message from the client
			recv(clientSocket, buffer, sizeof(buffer), 0);
			if(buffer[0] == SHUTDOWN){
				sem_wait(&serverBusyIndicator);
				for(int i = 0; i<NUM_RIDES; i++){
					f->rides[i].status = OFF_LINE;
				}
				sem_post(&serverBusyIndicator);
				break;
			}
			if(buffer[0] == ADMIT){
				sem_wait(&serverBusyIndicator);
				if(f->numGuests < MAX_GUESTS){
					f->numGuests++;
					response[0] = YES;
					// Create a new buffer that contains the response and the array of rides
					unsigned char sendBuffer[sizeof(response) + sizeof(f->rides)];
					memcpy(sendBuffer, response, sizeof(response));
					memcpy(sendBuffer + sizeof(response), f->rides, sizeof(f->rides));
					// Send the new buffer to the client
					send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
					sem_post(&serverBusyIndicator);
				}else{
					response[0] = NO;
					send(clientSocket, response, sizeof(response), 0);
					sem_post(&serverBusyIndicator);
					break;
				}
			}
			if(buffer[0] == GET_WAIT_ESTIMATE){ // (# guests waiting in line)/(ride’s capacity) * (ride’s combined running/loading/unloading time)
				sem_wait(&serverBusyIndicator);
				unsigned int waitEstimate;
				waitEstimate = (f->rides[buffer[1]].lineupSize/f->rides[buffer[1]].capacity) * (f->rides[buffer[1]].rideTime + ((f->rides[buffer[1]].onOffTime*2)*f->rides[buffer[1]].capacity));
				response[0] = waitEstimate;
				send(clientSocket, response, sizeof(response), 0);
				sem_post(&serverBusyIndicator);
			}
			if(buffer[0] == GET_IN_LINE){
				sem_wait(&serverBusyIndicator);
				if(f->rides[buffer[1]].lineupSize < MAX_LINEUP){
					response[0] = YES;
					response[1] = f->rides[buffer[1]].ticketsRequired;
					f->rides[buffer[1]].waitingLine[f->rides[buffer[1]].lineupSize] = buffer[2];
					f->rides[buffer[1]].lineupSize++;
				}else{
					response[0] = NO;
				}
				send(clientSocket, response, sizeof(response), 0);
				sem_post(&serverBusyIndicator);
				break;
			}
			if(buffer[0] == LEAVE_FAIR){
				sem_wait(&serverBusyIndicator);
				f->numGuests--;
				sem_post(&serverBusyIndicator);
				break;
			}

		}
		close(clientSocket);
		if(buffer[0] == SHUTDOWN){
			break;
		}
	}
	// Don't forget to close the sockets!
	close(serverSocket);
	printf("SERVER: Shutting down.\n");
	pthread_exit(NULL);
}
