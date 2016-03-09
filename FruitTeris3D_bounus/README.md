# Furit-Teris
* CMPT 361 Assignment 1
* Created by Jiaxun He(Victor)
* Furit-teris-3d is a 3D game where the teris is made of fruits and be placed in 3d board by a robort arm.

## Compile and run
* make
* ./FruitTeris3D

## How to play
* 'r' -> restart
* 'q' -> quit
* 'p' -> pause
* 'a' -> raise lower_arm
* 'd' -> lower lower_arm
* 'w' -> raise upper_arm
* 's' -> lower upper_arm
* 'z' -> rotate the whole arm along Y axis counter-clockwise
* 'x' -> rotate the whole arm along Y axis clockwise
* 'up' -> rotate along Z axis
* 'left' -> rotate along X axis
* 'right' -> rotate along Y axis

## Compeletion Status
* As far as I can see, I implemented all the features that are required
* I have finished the <strong>BOUNUS<strong> part
* Additional features:
	* pause game
	* let tile rotate along any axies

## Game Logic

### Titles
* The title, with random fruits, appears in random position, with random rotation, on the top of the board.
	* Initial position will be adjusted if out of board, to make it fit in the board
	* Initial position won't be adjusted if it collides with existing title settled in board
* Titles has 4 possible shapes
	* L shape
	* I shape
	* Z shape
	* T shape
	* 3 3D shapes
* Title has a lot of rotations for each shape, can rotate along:
	* X axis
	* Y axis
	* Z axis
* Collision
	* Collison is allowed when operating the robort arm
	* Once tile is settled (timer runs up or space key pressed), collison is not allowed
		* Game is over at that time
	* The fruit that has collision with other tile, or is out of board, is colored grey
* Eliminate
	* there is no elimination in this game, since it is not required
* Timer
	* timer count down will show up at the top of the screen
* Gameover
	* "gameover" will show up at the top of the screen, if it is the time
* Pause
	* timer will stop
	* no action is allowed
	* a string "paused" will show up at the top of the screen 

