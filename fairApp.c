#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>

#include "fair.h"
sem_t serverBusyIndicator;
#include "ride.c"
#include "requestHandler.c"
#include "display.c"

// Initialize a ride
void initializeRide(Ride *r, char *n, unsigned char tr, unsigned char cap,  unsigned char onOff, unsigned short rt, unsigned short wt) {
	r->name = n;
	r->ticketsRequired = tr;
	r->capacity = cap;
	r->onOffTime = onOff;
	r->rideTime = rt;
	r->waitTime = wt;
	r->lineupSize = 0;
	r->riders = (unsigned int *) malloc(sizeof(unsigned int) * r->capacity);
	if (r->riders == NULL) {
		printf("FAIR APP: Error allocating array for riders\n");
		exit(-1);
	}
	r->numRiders = 0;
	r->status = STOPPED;
}

// This is where it all begins
int main() {
	// Create a fair with no guests
	Fair	ottawaFair;
	ottawaFair.numGuests = 0;

	sem_init(&serverBusyIndicator, 0, 1);

	// Fill in the fair's ride information
	initializeRide(&(ottawaFair.rides[9]), "Ferris Wheel",   4, 32, 10, 600, 120);
	initializeRide(&(ottawaFair.rides[8]), "Pirate Ship",    4, 30,  5, 240, 120);
	initializeRide(&(ottawaFair.rides[7]), "Merry-Go-Round", 2, 25,  4, 240,  60);
	initializeRide(&(ottawaFair.rides[6]), "Roller Coaster", 5, 24,  5,  75,  60);
	initializeRide(&(ottawaFair.rides[5]), "Fun World",      1, 20,  1, 180,   1);
	initializeRide(&(ottawaFair.rides[4]), "Calm Train",     3, 16,  6, 300,  60);
	initializeRide(&(ottawaFair.rides[3]), "Back Destroyer", 4, 12, 10,  90,  30);
	initializeRide(&(ottawaFair.rides[2]), "Tea Cups",       3, 10, 10, 120,  60);
	initializeRide(&(ottawaFair.rides[1]), "Drop To Death",  5,  8, 20,  20,  30);
	initializeRide(&(ottawaFair.rides[0]), "Tummy Tosser",   5,  6,  7,  60,  30);

	// Start up the ride threads
	pthread_t rideT1, rideT2, rideT3, rideT4, rideT5, rideT6, rideT7, rideT8, rideT9, rideT10;
	pthread_create(&rideT1, NULL, runRide, &(ottawaFair.rides[0]));
	pthread_create(&rideT2, NULL, runRide, &(ottawaFair.rides[1]));
	pthread_create(&rideT3, NULL, runRide, &(ottawaFair.rides[2]));
	pthread_create(&rideT4, NULL, runRide, &(ottawaFair.rides[3]));
	pthread_create(&rideT5, NULL, runRide, &(ottawaFair.rides[4]));
	pthread_create(&rideT6, NULL, runRide, &(ottawaFair.rides[5]));
	pthread_create(&rideT7, NULL, runRide, &(ottawaFair.rides[6]));
	pthread_create(&rideT8, NULL, runRide, &(ottawaFair.rides[7]));
	pthread_create(&rideT9, NULL, runRide, &(ottawaFair.rides[8]));
	pthread_create(&rideT10, NULL, runRide, &(ottawaFair.rides[9]));

	// Spawn a thread to handle incoming requests from guests
	pthread_t guest_thread;
	pthread_create(&guest_thread, NULL, handleIncomingRequests, &ottawaFair);


  	// Spawn a thread to handle display
	pthread_t display_thread;
	pthread_create(&display_thread, NULL, showSimulation, &ottawaFair);

    	// Wait for the incoming requests thread to complete, from a STOP command
	pthread_join(guest_thread, NULL);

	// Shutdown the ride threads and free up the riders arrays
	pthread_join(rideT1, NULL);
	pthread_join(rideT2, NULL);
	pthread_join(rideT3, NULL);
	pthread_join(rideT4, NULL);
	pthread_join(rideT5, NULL);
	pthread_join(rideT6, NULL);
	pthread_join(rideT7, NULL);
	pthread_join(rideT8, NULL);
	pthread_join(rideT9, NULL);
	pthread_join(rideT10, NULL);

	for(int i = 0; i < NUM_RIDES; i++){
		for(int j = 0; j < ottawaFair.rides[i].numRiders; j++){
			printf("ottawaFair.rides[i].riders[j]: %u\n", ottawaFair.rides[i].riders[j]);
			kill(ottawaFair.rides[i].riders[j], SIGTERM);
			free(&(ottawaFair.rides[i].riders[j]));
		}
	}

	// Kill all the guest processes for any guests remaining
	pthread_cancel(display_thread);
	pthread_join(display_thread, NULL);
  	printf("FAIR APP: ended successfully\n");
}
