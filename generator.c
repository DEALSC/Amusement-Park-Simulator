#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "fair.h"

void main() {
  // Set up the random seed
  srand(time(NULL));

  // Startup 100 guests with ...
  // - a random number of tickets in the range of 5 to 40
  // - a random willing-to-wait time in the range of 600 to 900 seconds
  // - a random ride to go on first in teh range of 0 to NUM_RIDES
  unsigned char guestTickets;
  short guestWait;
  unsigned char priorityRide;
  char buffer[50];

  for (int i=0; i<100; i++) {
    guestTickets = rand()%36 + 5;
    guestWait = rand()%301 + 600;
    priorityRide = rand()%NUM_RIDES;
    sprintf(buffer, "./guest %d %d %d&", guestTickets, guestWait, priorityRide);
    printf("%s", buffer);
    system(buffer);

    usleep(10000); // Keep this 1/100 second delay between guest creation
  }
}
