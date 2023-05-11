# Amusement-Park-Simulator
Rider Ridden Ride Rides Ride

To start off you want to make sure you have the X Windows Systems before you can run the make file.

To install the X11 on debian-based systems use: sudo apt-get install libx11-dev
for all else, I'm not sure but you can search online or ask Chat.
  
Once the X11 is installed you can run 'make'

You'll now have 4 executables

[fairApp]

[guest]

[generator]

[stop]


You'll want to run fairApp in the background ``./fairApp&`` which should just pop up the graphic interface.
![image](https://github.com/DEALSC/Amusement-Park-Simulator/assets/83243290/067d864b-327a-4f1a-9bc3-1d73d798ddde)

So that started all the ride and requestHandler thread which then you can either run:

[guest: takes the format ``./guest <ticket> <wait time> <ride number>``]

<ticket> is the amount of tickets the guest will contain
  
<wait time> is the amount of time the guest that determines if the guest will wait in line
  
<ride number> a number between 0-9 which will be the guest's first ride if available else the next ride is randomly selected
  
ex. ``./guest 25 900 0``
  
[generator: randomly generates 100 guest to enter the fair]
  
[stop: shutdown the fair]

  https://github.com/DEALSC/Amusement-Park-Simulator/assets/83243290/f2ebe7e1-ce51-4409-b5da-3663dacb2085



  
