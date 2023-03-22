=========================================================
README
Starship Gold
=========================================================
Build & Run:
Open the solution using Visual Studio and Build the game.
Open Starship_x64.exe under Run folder to run the game.


=========================================================
Key board Controls:

Attract Mode:
	ESC - 	       exit application
	W / S -        toggle between menu options
	Spacebar / N - select menu option (start game / quit application)

Game Play:
	E - 	   accelerate player starship in forward direction
	S - 	   turn player starship to the left
	F - 	   turn player starship to the right
	Spacebar - fire a bullet at nose position
	N -  	   respawn player starship if destroyed
	ESC - 	   return to attract mode

Developer Hack During Gameplay:
	T -  slow time to 1/10 of the original (press and hold)
	O -  step one frame and pause the game
	P -  pause or unpause the game
	I -  spawn a random asteroid
	F1 - display debugging view
	F5 - increase player bullets per attack by 1
	F6 - decrease player bullets per attack by 1
	F8 - reset the game


========================================================
Xbox Controller Controls:

Attract Mode:
	BACK - 	        exit application
	Left Joystick - toggle between menu options
	A / Start - 	select menu option (start game / quit application)

Game Play:
	Left Joystick - turn player starship and accelerate in forward direction
	A -		fire a bullet at nose position
	BACK - 		return to attract mode
	START -  	respawn player starship if destroyed

Developer Hack During Gameplay:
	B -  pause or unpause the game
	X -  slow time to 1/10 of the original (press and hold)
	Y -  display debugging view
	RB - increase player bullets per attack by 1
	LB - decrease player bullets per attack by 1


========================================================
Known Bugs/Issues: None


========================================================
Deep Learning:
	
Every time I need to write a functionality for the game, I try to write the code from scratch. 
It always took me a while to realize there exists the function I need in the Engine code. 
The most recent example is when I tried to implement multiple bullets for the player ship, and I needed to calculate the new spawn locations of the bullets. 
I tried to calculate the sine and cosine and it was never right, the bullets stacked on top of each other when the player ship faces up or down. 
After an hour of grinding, I realized Vec2 has the function to rotate 90 or -90 degrees, so just rotating the forward normal will get the position of each tip of the player ship. 
If I think of it sooner, I could have saved time for other tasks. 
It feels like you have this huge collection of tools in the garage, but you decided to build a house with your bare hands. 
So my insight is to know your tool, and when you encounter a problem, always think if your Engine code already has the ability to solve it first. 
It is really crucial to know the potential of your Engine, because it will be used for every single game. 
This also applies to using commercial engines like Unity and Unreal, knowing what they can do really helps to reduce a ton of unnecessary head scratches.