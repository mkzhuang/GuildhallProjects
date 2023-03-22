===================================================================
README
Doomenstein
===================================================================
Build & Run:
Open the solution using Visual Studio and Build the game.
Open Doomenstein_x64.exe under Run folder to run the game.


===================================================================
Note:
	- unnecessary geometry are not rendered (not visible to player inside map)


===================================================================
Key board Controls:

Attract Mode:
	ESC - 	       exit application
	N/Space Bar -  join lobby with keyboard and mouse

Lobby Mode:
	ESC -  	       unjoin / exit to attract mode
	Spacer Bar -   join / start game

Game Play:
	ESC - 	   	return to attract mode
	W - 	   	move forward in looking direction
	S - 	   	move backward in looking direction
	A - 	   	move left of the looking direction
	D - 	   	move right of the looking direction
	Z - 		move up in z-axis
	C - 		move down in z-axis
	Mouse x-axis -  look left or right
	Mouse y-axis -	look up or down
	Mouse Left   -  fire weapon
	Q - 		rotate camera to left
	E - 		rotate camera to right
	F - 		Toggle free fly cam
	N - 		Possess next actor
	1 - 		Equip weapon at slot 1: pistol
	2 - 		Equip weapon at slot 2: plasma rifle
	3 -		Equip weapon at slot 3: shrink gun
	<- - 		Equip previous weapon
	-> - 		Equip next weapon
	Shift -		speed up movement by a scale
	9 - 		decrease debug clock speed
	0 - 		increase debug clock speed


Developer Hack During Gameplay:
	T -  slow time to 1/2 of the original (press and hold)
	Y -  speed time to 2 times the original (press and hold)
	O -  step one frame and pause
	P -  pause or unpause the game
	~ -  open developer console
	F8 - reset the game


==================================================================
Xbox Controller Controls:

Attract Mode:
	Back - 	        exit application
	A / Start - 	join lobby with controller

Lobby Mode:
	Back -		unjoin / exit to attract mode
	Start - 	join / start game

Game Play:
	BACK - 		     return to attract mode
	LeftStick x-axis -   move forward/backward in looking direction
	LeftStick y-axis -   move left/right of the looking direction
	RightStick x-axis -  look left or right
	RightStick y-axis -  look up or down
	Left Shoulder -	     speed up movement by a scale
	Right Shoulder -     fire weapon
	X - 		     equipe previous weapon
	Y - 		     equipe next weapon
	Start -		     reset camera to origin


==================================================================
Dev Console Commands:
**All gameplay inputs will be consumed when dev console is opened.**
	clear - 		 clear dev console screen
	help [Filter="filter"] - display all commands and events
	keys - 			 show all control keys
	debugRenderClear -	 clear all debug shapes and texts
	debugRenderToggle - 	 toggle all debug shapes and texts
	raycastDebugToggle - 	 toggle the ability to raycast


==================================================================
Known Bugs/Issues: None


==================================================================