#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>


// This code runs the ride forever ... until the process is killed
void *runRide(void *p) {
	Ride *r = (Ride *)p;
	r->countdownTimer = 0;

	// Go into an infinite loop to process the guests in line
	while(r->status != OFF_LINE) {
  		switch (r->status) {
			case STOPPED:
				sem_wait(&serverBusyIndicator);
				if(r->lineupSize == 0){
					if(r->numRiders > 0){
						r->countdownTimer--;
					}else{
						r->countdownTimer = 0;
					}
				}else{
					r->countdownTimer = r->onOffTime;
					r->status = LOADING;
				}
				if(r->countdownTimer == 0 && r->numRiders > 0){
					r->countdownTimer = r->rideTime;
					r->status = RUNNING;
				}
				sem_post(&serverBusyIndicator);
				break;
			case LOADING:
				sem_wait(&serverBusyIndicator);
				if(r->countdownTimer > 0){
					sem_post(&serverBusyIndicator);
					r->countdownTimer--;
					break;
				}
				// Remove the first guest from the waitingLine and add them to the riders array
				r->riders[r->numRiders] = r->waitingLine[0];
				r->numRiders++;
				r->lineupSize--;
				// Shift all values in the waitingLine over by 1
				for(int j = 0; j < r->lineupSize; j++) {
					r->waitingLine[j] = r->waitingLine[j+1];
				}
				// Send a SIGUSR1 signal to the guest process to indicate they have entered the ride
				kill(r->riders[r->numRiders-1], SIGUSR1);

				if(r->capacity == r->numRiders){
					r->countdownTimer = r->rideTime;
					r->status = RUNNING;
					sem_post(&serverBusyIndicator);
					break;
				}
				if(r->lineupSize == 0){
					r->countdownTimer = r->waitTime;
					r->status = STOPPED;
					sem_post(&serverBusyIndicator);
					break;
				}
				r->countdownTimer = r->onOffTime;
				sem_post(&serverBusyIndicator);
				break;
			case RUNNING:
				sem_wait(&serverBusyIndicator);
				r->countdownTimer--;
				if(r->countdownTimer == 0){
					r->countdownTimer = r->onOffTime;
					r->status = UNLOADING;
				}
				sem_post(&serverBusyIndicator);
				break;
			case UNLOADING:
				sem_wait(&serverBusyIndicator);
				while(r->countdownTimer > 0){
					r->countdownTimer--;
					sem_post(&serverBusyIndicator);
					break;
				}
				// Send a SIGUSR2 signal to the guest process to indicate they have exited the ride
				kill(r->riders[r->numRiders-1], SIGUSR2);
				//printf("Ride unloading guest %u off the ride\n", r->riders[r->numRiders-1]);
				r->numRiders--;
				r->countdownTimer = r->onOffTime;
				if(r->numRiders == 0){
					r->countdownTimer = r->waitTime;
					r->status = STOPPED;
				}
				sem_post(&serverBusyIndicator);
				break;
		}

		// Do not remove this line ... but you can put a large number here to slow things down a bit.
		usleep(10000);  // A 1/100 second delay to slow things down a little
	}
	printf("ride shutdown\n");
	pthread_exit(NULL);
}
