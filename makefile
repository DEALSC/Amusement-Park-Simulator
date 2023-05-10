all:		fairApp generator guest stop
		gcc -o fairApp fairApp.o -lX11 -lpthread
		gcc -o generator generator.o
		gcc -o guest guest.o
		gcc -o stop stop.o

fairApp:	fairApp.c display.c ride.c requestHandler.c fair.h display.h
		gcc -c fairApp.c

generator:	generator.c fair.h
		gcc -c generator.c

guest: 		guest.c fair.h
		gcc -c guest.c

stop:		stop.c fair.h
		gcc -c stop.c

clean:
		rm -f fairApp.o generator.o guest.o stop.o fairApp guest generator stop

