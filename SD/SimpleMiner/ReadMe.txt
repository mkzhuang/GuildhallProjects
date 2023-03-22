===================================================================
README
SimpleMiner
===================================================================
Build & Run:
Open the solution using Visual Studio and Build the game.
Open SimpleMiner_x64.exe under Run folder to run the game.

===================================================================
Profiling Results:
			FPS chunks activation		FPS stable
Debug				 45			  149
DebugInline			 46			  154
FastBreak		        112			  616
Release			        413			 1020

			       Worst			 Average
Chunk Gen		       2.27ms			 1.44ms
Disk Load		       0.49ms			 0.18ms
Disk Save		       0.58ms			 0.42ms
Chunk Rebuild		       3.78ms			 0.40ms
Light Resolve		       0.54ms			 0.00ms

===================================================================
Key board Controls:

Attract Mode:
	ESC - 	       exit application
	N - 	       start game

Game Play:
	ESC - 	   	return to attract mode
	W - 	   	move forward in looking direction
	S - 	   	move backward in looking direction
	A - 	   	move left of the looking direction
	D - 	   	move right of the looking direction
	Q - 		move up in z-axis in fly / no clip / spectator mode
	E - 		move down in z-axis fly / no clip / spectator mode
	Space Bar -	jump
	Mouse x-axis -  look left or right
	Mouse y-axis -	look up or down
	Shift -		speed up movement by a scale
	H -		reset camera to origin
	R - 		lock & unlock raycast
	1 ~ 9 -		change holding blocks
	Left Mouse - 	remove block 
	Right Mouse -	place holding block
	F2 -		change camera mode
	F3 -		change physics mode

Developer Hack During Gameplay:
	T -  slow game time to 1/2 of the original (press and hold)
	Y -  speed world time to 50 times the original (press and hold)
	O -  step one frame and pause
	P -  pause or unpause the game
	~ -  open developer console
	F1 - toggle debugging mode
	F8 - reset the world


==================================================================
Xbox Controller Controls:

Attract Mode:
	BACK - 	        exit application
	A / Start - 	start game

Game Play:
	BACK - 		     return to attract mode
	LeftStick x-axis -   move forward/backward in looking direction
	LeftStick y-axis -   move left/right of the looking direction   
	RightStick x-axis -  look left or right
	RightStick y-axis -  look up or down
	Left Trigger -	     move up in z-axis
	Right Trigger -	     move down in z-axis
	A -		     speed up movement by a scale
	Start -		     reset camera to origin


==================================================================
Dev Console Commands:
**All gameplay inputs will be consumed when dev console is opened.**
	clear - 		 clear dev console screen
	help [Filter="filter"] - display all commands and events
	keys - 			 show all control keys
	debugRenderClear -	 clear all debug shapes and texts
	debugRenderToggle - 	 toggle all debug shapes and texts


==================================================================
Known Bugs/Issues: None
	
==================================================================