#Multiplayer Serial Port ARDUITRON#

Description: Implementation of the TRON lightcycle game for multiple players, using the Serial port for communication. Both players should be able to see, and effectively crash into, each other's walls and driving of the cars will be realtime.

##MileStones##
1. Implement single-player, single-car tron. The car should move around the screen, keeping tracks of where it has been both visually through the trail left behind, and programmatically through an array of bits stored in memory. Crashing into the car's own walls or the screens boundaries will trigger failure.

2. Create a multiplayer spec and protocol to send data from one arduino to the other, as well as recieve it. Both cars should be drawn on both screens and can interact. There should be a countdown to start the match, either through physical lights on arduino hardware, or drawn on the screen.

3. Enhance the protocol to allow for extra setup from each player, such as car colour/player names.

4. Further extend the protocol to allow for up to four players through networked arduinos.

5. Implement a high score system. Because of the un-networked state of the arduinos, this system must be somewhat 'virusy' in nature, implemented as a type of mesh network. At startup the arduino would check against the local databases of scores of all the networked arduinos. From there, comparisons should be made and the high scores would self-propogate around every connected arduino before the beginning of the game.

6. Extend as far as possible, adding unique gameplay elements such as a potentiometer 'boost' that would cause a burst of speed, but cause a slower-moving cooldown for more interesting games.

##Deilvery and Demonstration##

The entirety of the codebase will remain available on github for anyone to check out as desired. The README.md will contain wiring information and game instructions for those unfamiliar with it.

Like the score system the game itself will be demonstrated frequently through the self-propogating, virus-like awesomeness that is TRON. Everyone able to play the lightcycle game will inevitibly do so.
