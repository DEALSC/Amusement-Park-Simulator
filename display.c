#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "display.h"

#define  GUEST_RADIUS    5

const int   TEXT_COLOR	     = 0x000000;
const int   BORDER_COLOR     = 0x000000;
const int   WAITING_COLOR    = 0xDD0000;
const int   LOADING_COLOR    = 0xCCAA00;
const int   UNLOADING_COLOR  = 0x0000DD;
const int   RIDING_COLOR     = 0x00EE00;


//Display-related variables
Display *display;
Window   win;
GC       gc;


// Initialize and open the simulator window with size ENV_WIDTH x ENV_HEIGHT.
void initializeWindow() {
  	// Open connection to X server
  	display = XOpenDisplay(NULL);

  	// Create a simple window, set the title and get the graphics context then
  	// make is visible and get ready to draw
  	win = XCreateSimpleWindow(display,  RootWindow(display, 0), 0, 0, ENV_WIDTH, ENV_HEIGHT, 0, 0x000000, 0xFFFFFF);
  	XStoreName(display, win, "Fair Simulator");
  	gc = XCreateGC(display, win, 0, NULL);
  	XMapWindow(display, win);
  	XFlush(display);
  	usleep(20000);  // sleep for 20 milliseconds.
}

// Close the display window
void closeWindow() {
  	XFreeGC(display, gc);
  	XUnmapWindow(display, win);
  	XDestroyWindow(display, win);
  	XCloseDisplay(display);
}


// Redraw all the ride inform and lineups repeatedly
void *showSimulation(void *x) {
	Fair	*f = (Fair *)x;
	Ride  	*rides = f->rides;
	char 	 status[20];	// holds a ride's status and countdown time
	
	initializeWindow();
	
	// Draw the line dividers
  	XSetForeground(display, gc, BORDER_COLOR);
    for (int i=0; i<NUM_RIDES; i++) {
    	int x = 100 + rides[i].capacity * GUEST_RADIUS * 2 + GUEST_RADIUS;
		XDrawLine(display, win, gc, x, (i+1)*50-8, x, (i+1)*50-8+GUEST_RADIUS*2);
    }
	// Draw all the Ride labels
    for (int i=0; i<NUM_RIDES; i++) {
		XSetForeground(display, gc, TEXT_COLOR);
		XDrawString(display, win, gc, 10, (i+1)*50-3, rides[i].name, strlen(rides[i].name));
	}

	// Now keep redrawing until someone kills the thread
  	while(1) {
  		// Draw the Number of guests
  		XSetForeground(display, gc, 0xFFFFFF);
   		XFillRectangle(display, win, gc, 10, 0, 90, 30);
   		strcpy(status, "GUESTS: ");
   		sprintf(&(status[8]), "%3d", f->numGuests);
		XSetForeground(display, gc, TEXT_COLOR);
		XDrawString(display, win, gc, 10, 20, status, strlen(status));

		// Draw the Ride status and timer data
    	for (int i=0; i<NUM_RIDES; i++) {
			XSetForeground(display, gc, 0xFFFFFF);
   			XFillRectangle(display, win, gc, 10, (i+1)*50, 90, 15);
			
			if (rides[i].status == STOPPED)
				strcpy(status, "STOPPED   ");
			if (rides[i].status == LOADING)
				strcpy(status, "LOADING   ");
			if (rides[i].status == RUNNING)
				strcpy(status, "RUNNING   ");
			if (rides[i].status == UNLOADING)
				strcpy(status, "UNLOADING ");
			sprintf(&(status[10]), "%1d:%02d", rides[i].countdownTimer/60, rides[i].countdownTimer%60);
			XSetForeground(display, gc, TEXT_COLOR);
			XDrawString(display, win, gc, 10, 12+(i+1)*50, status, strlen(status));
    	}
    	
    	// Draw all the Ride Lineups
    	for (int i=0; i<NUM_RIDES; i++) {
    		unsigned int  rideSize = rides[i].capacity * GUEST_RADIUS * 2;
    		
    		// Determine the color to draw the riders
    		int color = LOADING_COLOR;
    		if (rides[i].status == RUNNING)
    			color = RIDING_COLOR;
    		else if (rides[i].status == UNLOADING)
    			color = UNLOADING_COLOR;
    		
    		// Now draw the riders
    		for (int g=0; g<rides[i].capacity; g++) {
    			if (g < rides[i].numRiders) 
    				XSetForeground(display, gc, color);
				else 
    				XSetForeground(display, gc, 0xFFFFFF);				
				XFillArc(display, win, gc, 100 + g*GUEST_RADIUS*2, (i+1)*50-8, 2*GUEST_RADIUS, 2*GUEST_RADIUS, 0, 360*64);
				XSetForeground(display, gc, BORDER_COLOR);
				XDrawArc(display, win, gc, 100 + g*GUEST_RADIUS*2, (i+1)*50-8, 2*GUEST_RADIUS, 2*GUEST_RADIUS, 0, 360*64);
			}
			// Now draw those waiting in line
    		for (int g=1; g<=MAX_LINEUP; g++) {
    			if (g <= rides[i].lineupSize) 
    				XSetForeground(display, gc, WAITING_COLOR);
				else 
    				XSetForeground(display, gc, 0xFFFFFF);
    			XFillArc(display, win, gc, 100 + rideSize + g*GUEST_RADIUS*2, (i+1)*50-8, 2*GUEST_RADIUS, 2*GUEST_RADIUS, 0, 360*64);
				XSetForeground(display, gc, BORDER_COLOR);
				XDrawArc(display, win, gc, 100 + rideSize + g*GUEST_RADIUS*2, (i+1)*50-8, 2*GUEST_RADIUS, 2*GUEST_RADIUS, 0, 360*64);
			}
		}
    	XFlush(display);
    	usleep(2000);
	}
  	closeWindow();
  	pthread_exit(NULL);
}


