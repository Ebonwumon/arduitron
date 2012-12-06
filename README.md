#Multiplayer Serial Port ARDUITRON#

Description: Implementation of the TRON lightcycle game for multiple players, using the Serial port for communication. Both players should be able to see, and effectively crash into, each other's walls and driving of the cars will be realtime.

##Wiring##

###LCD###
The wiring for the LCD screen is as per the assignment 2 spec, however the LCD screen must be oriented such that the SD card slot on the screen faces *away* from the Arduino chip. 

###Joystick###
Vertical --> A0
Horizontal --> A1
Button/Sel --> Pin 9

Ground/Power as per normal.

###Lights###
First (Red) --> 22
Second (Red) --> 24
Third (Green) --> 26
Fourth (Red) --> 28
Fifth (Red) --> 30

*NOTE* Ensure there are resistors on each LED or else the entire world will explode. 

##Build Notes##
Your build target is tron.cpp, with the lcd image libraries. You will need two arduinos to sufficiently run this project; there is no single player mode.

For startup graphics tron.lcd will need to be put on the root of the SD card - if it's not there the code *should* work fine imageless, but there is no guarantee of that.

##Running##
There are a couple expectations of the user in this application. First, the user must not interact with the joystick before bootup has finished (don't touch the joystick until you see arduitron image and instructions to continue). Wait until both arduinos have booted before pressing the joystick. If both haven't started you may get timeout errors on connecting.

Sometimes after extended play the arduinos serial connection will fall out of sync. This is self-corrected by the arduino, if they are out of sync and you attempt to start a new game you will get a black screen and no countdown timer. To fix this you must reset both arduinos. This most certainly a feature and not a bug (it emulates the insertion of a coin into an arcade machine).

EULA: By using this software you are implicitly agreeing that should you become sucked into the virtual game world you will collect and surrender unto the author of this software Jeff Bridges's autograph.

##MileStones##
1. Implement single-player, single-car tron. The car should move around the screen, keeping tracks of where it has been both visually through the trail left behind, and programmatically through an array of bits stored in memory. Crashing into the car's own walls or the screens boundaries will trigger failure.

2. Create a multiplayer spec and protocol to send data from one arduino to the other, as well as recieve it. Both cars should be drawn on both screens and can interact. There should be a countdown to start the match, either through physical lights on arduino hardware, or drawn on the screen.

3. Enhance the protocol to allow for extra setup from each player, such as car colour/player names.

4. Further extend the protocol to allow for up to four players through networked arduinos.

5. Implement a high score system. Because of the un-networked state of the arduinos, this system must be somewhat 'virusy' in nature, implemented as a type of mesh network. At startup the arduino would check against the local databases of scores of all the networked arduinos. From there, comparisons should be made and the high scores would self-propogate around every connected arduino before the beginning of the game.

6. Extend as far as possible, adding unique gameplay elements such as a potentiometer 'boost' that would cause a burst of speed, but cause a slower-moving cooldown for more interesting games.

##Deilvery and Demonstration##

The entirety of the codebase will remain available on [github](https://github.com/Ebonwumon/arduitron) for anyone to check out as desired. The README.md will contain wiring information and game instructions for those unfamiliar with it.

Like the score system the game itself will be demonstrated frequently through the self-propogating, virus-like awesomeness that is TRON. Everyone able to play the lightcycle game will inevitibly do so.
