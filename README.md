# Furit-Teris
* CMPT 361 Assignment 1
* Created by Jiaxun He(Victor)
* Used skeleton code provied by the TA
* Furit-teris is a game where the teris is made of fruits and can be eliminated by the rules in falling fruit!

## Compile and run
* make
* ./FruitTeris

## How to play
* 'r' -> restart
* 'q' -> quit
* 'p' -> pause
* 'a' -> accelerate
* 'up' -> rotate
* 'down' -> move title down by 1
* 'left' -> move title to the left by 1
* 'right' -> move title to the right by 1

## Compeletion Status
* As far as I can see, I implemented all the features that are required.
* Additional features:
	* pause game
	* accelerate falling speed


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
* Title has 4 possible rotations for each shape
	* Even I and Z has 4 rotations.
	* I made it so because they are different due to the fruits in it
* Change rotations
	* Press up arrow key to rotate, in counter-clockwise direction.
	* if no collison after rotation, rotate it
	* If collison, I will move it by 1 horizontally to try to find a position where there is no collision. If still doesn't work, then title will not be rotated.
* Falling Speed 
	* the initial falling interval is 1000ms
	* press 'a' and interval will decrese 200ms
	* minimum interval is 200ms
	* no way to increse the interval except for restarting the game
* Eliminate
	* If there is a full row, it will be eliminated
	* If there is more than 3 consecutive same fruits, they will be eliminated.
	* Real elimination will happen when all tests is done, which means that the maximum fruits will be eliminated.
	* And elimination will go on untill no fruits can be eliminated.
	* All the fruit will move done by the number of fruits down them that are eliminated.
* Game termination
	* The game will halt when it's over. And will prompt on stdout that the game is over.

