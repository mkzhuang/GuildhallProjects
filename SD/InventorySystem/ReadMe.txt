===================================================================
README
InventorySystem
===================================================================
Build & Run:
Open the solution using Visual Studio and Build the game.
Open InventorySystem_x64.exe under Run folder to run the game.

App flow:
	Run the app, go to gameplay screen, press I to open inventory menu,
	press K near chest to open.

===================================================================
Key board Controls:

Attract Mode:
	ESC - 	       exit application
	N/Space -      start game

Game Play:
	I - 		toggle inventory
	K -		toggle chest if nearby
	mousewheel -	scroll items if inventory is open

	ESC - 	   	return to attract mode
	WASD -		movement


Developer Hack During Gameplay:
	T -  slow time to 1/2 of the original (press and hold)
	Y -  speed time to 2 times the original (press and hold)
	O -  step one frame and pause
	P -  pause or unpause the game
	~ -  open developer console
	F1 - toggle debugging mode (nothing just yet)
	F8 - reset the game


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