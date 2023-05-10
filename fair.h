#define		NUM_RIDES			 10  // Number of rides at the fair
#define		RIDE_NAME_MAX_CHARS	 20  // Maximum number of characters in a ride's name, including the '\0'
#define		MAX_GUESTS			300  // Maximum number of guests allowed at the fair
#define		MAX_LINEUP			 50  // Maximum Number of guests that can wait in a lineup


// Various modes that a Ride may be in at any time
#define		OFF_LINE			0
#define		STOPPED				1
#define		LOADING				2  // time = capacity*2
#define		RUNNING				3  // time = Fixed
#define		UNLOADING			4  // time = capacity*2


// Responses
#define		NO				0
#define		YES				1


// Message requests that a Guest makes to a Fair
#define		SHUTDOWN			0	// No data --> OK
#define		ADMIT				1	// No data --> list of ride names, ride IDs, ticketRequirement
#define		GET_WAIT_ESTIMATE	2	// ride ID --> wait time estimate (-1 if invalid ride ID)
#define		GET_IN_LINE			3	// ride ID, processID --> OK or NO (if full)
#define		LEAVE_FAIR			4	// processID --> NO RESPONSE

// Settings for Fair Server setup
#define SERVER_IP     "127.0.0.1"   // IP address of simulator server
#define SERVER_PORT          6000   // PORT of the simulator server


// This struct represents a Ride that the guests can go on
typedef struct  {
	int		 		 pid;						// Process ID for this ride
	char 			*name;						// Name of the ride
	unsigned char	 status;					// Status of the ride as either OFF_LINE, STOPPED, LOADING, RUNNING or UNLOADING
	unsigned char 	 ticketsRequired;			// Between 1 to 5 
	unsigned char	 capacity;					// Number of guests that can ride at the same time
	unsigned char	 onOffTime;					// Number of seconds that it takes for a single rider to get on/off the ride
	unsigned short	 rideTime;					// Number of seconds that the ride runs for each time
	unsigned short	 waitTime;					// Number of seconds that the ride waits before running, if not at full capacity
	unsigned int	 waitingLine[MAX_LINEUP];	// Lineup of guests for this ride (keep their process IDs)
	unsigned char	 lineupSize;				// Number of guests waiting in line
	unsigned int	*riders;					// Guests that are on the ride (keep their process IDs)
	unsigned char	 numRiders;					// Number of guests currently on the ride
	unsigned short	 countdownTimer;			// Number of seconds to count down until loading/unloading/running/waiting is done
} Ride;


// This struct represents a Fair with rides and guests
typedef struct {
	Ride			rides[NUM_RIDES];
	unsigned int	guestIDs[MAX_GUESTS];		// guest Process IDs ... needed to stop the guest processes after a shutdown
	unsigned int	numGuests;					// Number of admitted guests
} Fair;
