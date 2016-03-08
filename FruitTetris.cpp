/*
 * Jiaxun He (Victor)
 * 301295756
 *
 * CMPT 361 Assignment 1 
 *
 * - Compile and Run:
 * Type make in terminal, then type ./FruitTetris
 * 
 * Written by Jiaxun He, on the basis of the skeleton code provided by the TA, a lot of changes though
 *  
 * And the skeleton code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
 * by Rui Ma (ruim@sfu.ca) on 2014-03-04. 
 * Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <cstdlib>

using namespace std;

int timerIntervial = 1000;
int gameRound = 0;

bool halted = false;
bool paused = false;

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 600; 	// used to be 400
int ysize = 820;	// used to be 720
int zsize = 33;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilePos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// the type and rotation of the current tile
int tileType;
int rotationStatus;


// constants
static const int numOfGridPoints = 590; // 64 * 2 + 11 * 21 * 2

// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 10.0;
const GLfloat BASE_WIDTH       = 50.0;
const GLfloat LOWER_ARM_HEIGHT = 150.0;
const GLfloat LOWER_ARM_WIDTH  = 10;
const GLfloat UPPER_ARM_HEIGHT = 50.0;
const GLfloat UPPER_ARM_WIDTH  = 5;

// angel to control arm
GLfloat theta_arm = 0;
GLfloat phi_arm = 0;
mat4 armModel_view;

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] = {
	{vec2(0,0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
		{vec2(0,0), vec2(0,-1), vec2(0, 1), vec2(1, -1)},
		{vec2(0,0), vec2(1, 0), vec2(-1,0), vec2(1,  1)},
		{vec2(0,0), vec2(0, 1), vec2(0,-1), vec2(-1, 1)}
};


vec2 allRotationsShapes[4][4][4] = {
	// L shape
	{
		{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
		{vec2(0, 0), vec2(0,-1), vec2(0, 1), vec2(1, -1)},
		{vec2(0, 0), vec2(1, 0), vec2(-1,0), vec2(1,  1)},
		{vec2(0, 0), vec2(0, 1), vec2(0,-1), vec2(-1, 1)}
	},
	// I shape
	{
		{vec2(0, 0), vec2(1, 0), vec2(-1, 0), vec2(-2, 0)},
		{vec2(0, 0), vec2(0, 1), vec2(0, -1), vec2(0, -2)},
		{vec2(0, 0), vec2(-1,0), vec2(1,  0), vec2(2,  0)},
		{vec2(0, 0), vec2(0,-1), vec2(0,  1), vec2(0,  2)}
	},
	// S shape
	{
		{vec2(0, 0), vec2(1, 0), vec2(0, -1), vec2(-1,-1)},
		{vec2(0, 0), vec2(0, 1), vec2(1,  0), vec2(1, -1)},
		{vec2(0, 0), vec2(-1,0), vec2(0,  1), vec2(1,  1)},
		{vec2(0, 0), vec2(0,-1), vec2(-1, 0), vec2(-1, 1)}
	},
	// T shape
	{
		{vec2(0, 0), vec2(1, 0), vec2(-1, 0), vec2(0, -1)},
		{vec2(0, 0), vec2(0, 1), vec2(0, -1), vec2(1,  0)},
		{vec2(0, 0), vec2(-1,0), vec2(1,  0), vec2(0,  1)},
		{vec2(0, 0), vec2(0,-1), vec2(0,  1), vec2(-1, 0)}
	}
};


// board colors
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); 

// fruit colors
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);

vec4 fruitColors[5] = {orange, red, green, blue, purple};

// furit colors of current tile
vec4 currenttileFruitColors[4];

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200 * 6];

// An array contatinng all the points of three parts of the robit arm
vec4 armPoints[3*36];
// a base offset vec, which translates the roboat from origin
vec3 baseOffset = vec3(-40, 36, 0);

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;
GLuint loczsize;

// locations of shader transformation matrices
GLuint locM_V;
GLuint locP;
mat4 model_view;
mat4 projection;

// viewing transformation parameters
GLfloat radius = 1500;
GLfloat theta = 0.0;
GLfloat phi = 0.0;

// projection transformation parameters

GLfloat fovy = 45.0;	// Field-of0View in Y direction angle (degrees)
GLfloat aspect = 1.0*xsize/ysize;
GLfloat zNear = 10;
GLfloat zFar = 200;


// VAO and VBO
GLuint vaoIDs[4]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[8]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)


//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// fuc decalrations

// for sepcial keys(arrow keys)
void rotatetile();
void restartGame();

void moveTileToLeft();
void moveTileToRight();
bool moveTileDown();
void moveTileDownAndSettle();

void moveCameraClockwise();
void moveCameraCounterlockwise();

void displaytile();
void Timer(int);

void increaseTheta_Arm();
void decreaseTheta_Arm();
void increasePhi_Arm();
void decreasePhi_Arm();

//---------------------------------------------------------------------------------------------------------------------
// helper methods

// copy array of 4 vec2 from src to dst
void copyArray4OfVec2(vec2* dst, vec2* src) {
	for(int i = 0; i < 4; i++) {
		dst[i] = src[i];
	}
}

// generate random num between 0 and n-1
int randomNum(int n) {
	return rand()%n;
}

// return true if two vec4 is equal
bool isVec4Equal(vec4 a, vec4 b) {
	int flag = true;
	for (int i = 0; i < 4; ++i)
	{
		if (a[i] != b[i])
		{
			flag = false;
		}
	}
	return flag;
}


// update board vbo
void updateVboOfBoardColor() {
	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glBindVertexArray(0);
	glBindVertexArray(vboIDs[3]);
}


// get fruit color
vec4 getGridColor(int x, int y) {
	int vertexIndex = 6 * ( y * 10 + x);
	return boardcolours[vertexIndex];
}

vec4 getGridColor(vec2 pos) {
	int vertexIndex = 6 * ( pos.y * 10 + pos.x);
	return boardcolours[vertexIndex];
}

//----------------------------------------------------------------------------------------------------------------------
// tile operations

// return true if occupied or out of board
bool occupied(int x, int y) {
	if(x > 9 || x < 0) {
		// x out of bound
		return true;
	}
	if(y > 19 || y < 0) {
		// y out of bound
		return true;
	}

	return board[x][y];
}

// return true if collide, false otherwise
bool collide(vec2* tile, vec2 direction) {
	return occupied(tilePos.x + tile[0].x + direction.x, tilePos.y + tile[0].y + direction.y)
		|| occupied(tilePos.x + tile[1].x + direction.x, tilePos.y + tile[1].y + direction.y)
		|| occupied(tilePos.x + tile[2].x + direction.x, tilePos.y + tile[2].y + direction.y)
		|| occupied(tilePos.x + tile[3].x + direction.x, tilePos.y + tile[3].y + direction.y);
}

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool moveTile(vec2 direction) {
	// if not collide, move tile
	if(!collide(tile, direction)) {
		/*
		tilePos.x += direction.x;
		tilePos.y += direction.y;
		*/
		tilePos += direction;
		return true;
	}
	// otherwise return false
	return false;
}

// Places the current tile
void settleTile()
{
	// update the board vertex colour VBO
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 36; j++)
		{
			// each square has 6 vertex
			int vIndex = 36 * (10 * (tilePos.y + tile[i].y) + (tilePos.x + tile[i].x)) + j;
			boardcolours[vIndex] = currenttileFruitColors[i];
		}
	}

	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glBindVertexArray(0);
	glBindVertexArray(vboIDs[3]);

	// update occupied cells array
	for (int i = 0; i < 4; ++i)
	{
		int x = tilePos.x + tile[i].x;
		int y = tilePos.y + tile[i].y;
		board[x][y] = true;
	}
}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updateTile() {
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilePos.x + tile[i].x; 
		GLfloat y = tilePos.y + tile[i].y;

		// Create the 8 corners of the cubic - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1);

		vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 16.5, 1); 
		vec4 p6 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 16.5, 1);
		vec4 p7 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 16.5, 1);
		vec4 p8 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 16.5, 1);


		// Two points are used by two triangles(6 vertices) each
		// 6 faces, 6 * 6 vettices
		vec4 newpoints[6 * 6] = {p1, p2, p3, p2, p3, p4,	// front face
								 p5, p6, p7, p6, p7, p8,	// back face
								 p1, p2, p5, p2, p5, p6, 	// left face
								 p3, p4, p7, p4, p7, p8,	// right face
								 p1, p3, p5, p3, p5, p7,	// top face
								 p2, p4, p6, p4, p6, p8		// bottom
								}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*6*sizeof(vec4), 6*6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

// whether a tile is out of upper bound
// used when generating position of new tile
bool istileOutOfUpperBound() {
	for (int i = 0; i < 4; ++i)
	{
		if (tilePos.y + tile[i].y > 19)
		{
			return true;
		}
	}

	return false;
}

bool istileOutOfLeftBound() {
	for (int i = 0; i < 4; ++i)
	{
		if (tilePos.x + tile[i].x < 0)
		{
			return true;
		}
	}

	return false;
}

bool istileOutOfRightBound() {
	for (int i = 0; i < 4; ++i)
	{
		if (tilePos.x + tile[i].x > 9)
		{
			return true;
		}
	}

	return false;
}

// Called at the start of play and every time a tile is placed
void newTile()
{


	int tileXPos = randomNum(10);
	int tileRotation = randomNum(4);

	tilePos = vec2(tileXPos , 19); // Put the tile at the top of the board
	tileType = randomNum(4);	// random type of tile

	// Update the geometry VBO of current tile
	for (int i = 0; i < 4; i++) {
		tile[i] = allRotationsShapes[tileType][tileRotation][i]; // Get the 4 pieces of the new tile
	}

	// adjust position of tile to fit in the screen
	if (istileOutOfUpperBound())
	{
		tilePos.y += -1;
		if (istileOutOfUpperBound())
		{
			tilePos.y += -1;
		}
	}
	if (istileOutOfRightBound())
	{
		tilePos.x += -1;
		if (istileOutOfRightBound())
		{
			tilePos.x += -1;
		}
	}
	if (istileOutOfLeftBound())
	{
		tilePos.x += 1;
		if (istileOutOfLeftBound())
		{
			tilePos.x += 1;
		}
	}


	updateTile(); 


	// generate random color for each fruit
	for (int i = 0; i < 4; i++)
	{
		currenttileFruitColors[i] = fruitColors[randomNum(5)];
	}

}

void displaytile() {
	// Update the color VBO of current tile
	vec4 newcolours[24 * 6];

	for (int i = 0; i < 24 * 6; i++) {
		newcolours[i] = currenttileFruitColors[i/(6 *6)]; 
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
// game logic

// return true if game is over
bool isGameOver() {
	return collide(tile, vec2(0,0));
}



//-------------------------------------------------------------------------------------------------------------------
// game initialization


void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[numOfGridPoints]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[numOfGridPoints]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] 	= vec4((33.0 + (33.0 * i)), 33.0, 16.5, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 16.5, 1);

		gridpoints[64 + 2*i]	 = vec4((33.0 + (33.0 * i)), 33.0, -16.5, 1);
		gridpoints[64 + 2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, -16.5, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] 	 = vec4(33.0, (33.0 + (33.0 * i)), 16.5, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 16.5, 1);

		gridpoints[64 + 22 + 2*i] 	  = vec4(33.0, (33.0 + (33.0 * i)), -16.5, 1);
		gridpoints[64 + 22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), -16.5, 1);
	}

	// Lines along z axies
	for (int i = 0; i < 11; ++i)
	{
		for (int j = 0; j < 21; ++j)
		{
			gridpoints[128 + 22 * j + 2 * i] = vec4(33.0 + (i * 33.0), 33.0 + (j * 33.0), 16.5, 1);
			gridpoints[128 + 22 * j + 2 * i + 1] = vec4(33.0 + (i * 33.0), 33.0 + (j * 33.0), -16.5, 1);
		}
	}

	// 
	// Make all grid lines white
	for (int i = 0; i < numOfGridPoints; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, numOfGridPoints*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, numOfGridPoints*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void setCubicFace(vec4 *boardpoints, int startingIndex, 
				  vec4 p1, vec4 p2, vec4 p3, vec4 p4) {
	boardpoints[startingIndex] 	   = p1;
	boardpoints[startingIndex + 1] = p2;
	boardpoints[startingIndex + 2] = p3;
	boardpoints[startingIndex + 3] = p2;
	boardpoints[startingIndex + 4] = p3;
	boardpoints[startingIndex + 5] = p4;

}

void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200 * 6];
	for (int i = 0; i < 1200 * 6; i++) {
		boardcolours[i] = black; // Let the empty cells on the board be black
	}

	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), -16.5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), -16.5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), -16.5, 1);

			vec4 p5 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0),  16.5, 1); 
			vec4 p6 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0),  16.5, 1);
			vec4 p7 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0),  16.5, 1);
			vec4 p8 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0),  16.5, 1);

			int startingIndex = 36*(10*i + j);

			setCubicFace(boardpoints, startingIndex,    p1, p2, p3, p4);	// front
			setCubicFace(boardpoints, startingIndex+6,  p5, p6, p7, p8);	// back
			setCubicFace(boardpoints, startingIndex+12, p1, p2, p5, p6);	// left
			setCubicFace(boardpoints, startingIndex+18, p3, p4, p7, p8);	// right
			setCubicFace(boardpoints, startingIndex+24, p1, p3, p5, p7);	// top
			setCubicFace(boardpoints, startingIndex+30, p2, p4, p6, p8);	// bottom

			/*
			// Two points are reused
			// front
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
			*/

		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}



// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// init robot arm

// cube factory
//  width 1, centered at origin
vec4 *cubeWithUnitWidth() {

	vec4 *cube = new vec4[36];

	vec4 p1 = vec4(0, 0, -1, 1);
	vec4 p2 = vec4(0, 1, -1, 1);
	vec4 p3 = vec4(1, 0, -1, 1);
	vec4 p4 = vec4(1, 1, -1, 1);

	vec4 p5 = vec4(0, 0,  1, 1); 
	vec4 p6 = vec4(0, 1,  1, 1);
	vec4 p7 = vec4(1, 0,  1, 1);
	vec4 p8 = vec4(1,1,  1, 1);

	int startingIndex = 0;	// base

	setCubicFace(cube, startingIndex,    p1, p2, p3, p4);	// front
	setCubicFace(cube, startingIndex+6,  p5, p6, p7, p8);	// back
	setCubicFace(cube, startingIndex+12, p1, p2, p5, p6);	// left
	setCubicFace(cube, startingIndex+18, p3, p4, p7, p8);	// right
	setCubicFace(cube, startingIndex+24, p1, p3, p5, p7);	// top
	setCubicFace(cube, startingIndex+30, p2, p4, p6, p8);	// bottom

	return cube;
}

void base() {
	vec4 *cube = cubeWithUnitWidth();

	mat4 transformMat = mat4();
	// base offset from origin
	transformMat *= Translate(baseOffset);
	// scale to proper size
	transformMat *= Scale(BASE_WIDTH, BASE_HEIGHT, BASE_WIDTH);
	


	// do transform, stored in corresponding points to draw
	for (int i = 0; i < 36; ++i)
	{
		armPoints[i] = transformMat * cube[i];
	}

	delete []cube;
}

void lowerArm() {
	vec4 *cube = cubeWithUnitWidth();

	mat4 transformMat = mat4();
	// base offset from origin
	transformMat *= Translate(baseOffset);
	// translate
	transformMat *= Translate(BASE_WIDTH/2.0, BASE_HEIGHT/2,0);
	// rotate
	transformMat *= RotateZ(-theta_arm);
	// scale to proper shape
	transformMat *= Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH);

	// do transform, soted in correponding points to draw
	for (int i = 0; i < 36; ++i)
	{
		armPoints[36 + i] = transformMat * cube[i];
	}

	delete []cube;

}

void upperArm() {
	vec4 *cube = cubeWithUnitWidth();

	mat4 transformMat = mat4();
	// base offset
	transformMat *= Translate(baseOffset);
	// tranlation based on lower and uper arm
	transformMat *= Translate(BASE_WIDTH/2.0 + LOWER_ARM_HEIGHT*sin(DegreesToRadians*theta_arm),
							  BASE_HEIGHT/2.0 + LOWER_ARM_HEIGHT*cos(DegreesToRadians*theta_arm),
							  0);
	// rotate
	transformMat *= RotateZ(-theta_arm + phi_arm - 90);
	// scale to proper shape
	transformMat *= Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH);

	// do transformation, stored in corresponding points to draw
	for (int i = 0; i < 36; ++i)
	{
		armPoints[72 + i] = transformMat * cube[i];
	}

	delete []cube;

}

void drawArm() {

	base();
	lowerArm();
	upperArm();

}

void initRobortArm() {
	glBindVertexArray(vaoIDs[3]);
	glGenBuffers(2, &vboIDs[6]);

	drawArm();

	// Current arm vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 36*3*sizeof(vec4), armPoints, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);


	// arm colors
	vec4 armColors[36*3];
	for (int i = 0; i < 36; ++i)
	{
		armColors[i] = orange;
	}

	for (int i = 0; i < 36; ++i)
	{
		armColors[36 + i] = blue;
	}

	for (int i = 0; i < 36; ++i)
	{
		armColors[72 + i] = red;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[7]);
	glBufferData(GL_ARRAY_BUFFER, 36*3*sizeof(vec4),  armColors, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Get the location of the model_view_projection
	locM_V = glGetUniformLocation(program, "ModelView");
	locP = glGetUniformLocation(program, "Projection");


	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();
	initRobortArm();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");
	loczsize = glGetUniformLocation(program, "zsize");

	// Game initialization
	newTile(); // create new next tile
	displaytile();

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------


// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// projection
	projection = Perspective(fovy, aspect, zNear, zFar);

	// model_view
	model_view = Scale(2.0/33, 2.0/33, 2.0/33);

	vec4 eye(radius*sin(theta)*cos(phi),
			 radius*sin(theta)*sin(phi),//+ 500, 
			 radius*cos(theta), 
			 1.0);
	vec4 at(0, 20/2, 0, 1.0);
	vec4 up(0, 1, 0, 0);

	model_view *= LookAt(eye, at, up);

	glUniformMatrix4fv(locM_V, 1, GL_TRUE, model_view);
	glUniformMatrix4fv(locP, 1, GL_TRUE, projection);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);
	glUniform1i(loczsize, zsize);

	
	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200 * 6); // Draw the board (10*20*2 = 400 triangles)
	
	
	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24 * 6); // Draw the current tile (48 triangles)

	glBindVertexArray(vaoIDs[3]);	
	glDrawArrays(GL_TRIANGLES, 0, 36 * 3);	// Draw the roboat arm

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, numOfGridPoints); // Draw the grid lines (21+11 = 32 lines)
	


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	if (halted || paused)
	{
		return;
	}

	int mode = glutGetModifiers();
	if (mode == GLUT_ACTIVE_CTRL)
	{
		cout << "shit" << endl;
		switch(key) {
			case GLUT_KEY_LEFT: moveCameraCounterlockwise(); return;
			case GLUT_KEY_RIGHT:moveCameraClockwise(); return;
			default: break;
		}
	}

	switch(key) {
		case GLUT_KEY_UP: rotatetile(); break;	// rotate tile if "up"
		//case GLUT_KEY_LEFT: moveTileToLeft(); break;
		//case GLUT_KEY_RIGHT: moveTileToRight(); break;
		case GLUT_KEY_DOWN: moveTileDownAndSettle(); break;
		default: break;
	}
}

// special key actions

void moveCameraClockwise() {
	theta += 0.1;
}

void moveCameraCounterlockwise() {
	theta -= 0.1;
}

// Rotates the current tile, when up is pressed
void rotatetile() {      
	// new rotation
	int newRotation = rotationStatus;

	newRotation = (newRotation + 1) % 4;

	// check whether possible to rotate
	if(collide(allRotationsShapes[tileType][newRotation], vec2(0,0))) {
		// if the rotated tile will collide
		if(!collide(allRotationsShapes[tileType][newRotation], vec2(1,0))) {
			// if move to the right by 1 will work
			tilePos.x += 1;
		} else if(!collide(allRotationsShapes[tileType][newRotation], vec2(-1,0))) {
			// if move to the left by 1 will work
			tilePos.x += -1;
		} else if(!collide(allRotationsShapes[tileType][newRotation], vec2(2, 0))) {
			// if move to the right by 2 will work
			tilePos.x += 2;
		} else if(!collide(allRotationsShapes[tileType][newRotation], vec2(-2, 0))) {
			// if move to the left by 2 will work
			tilePos.x += -2;
		} else {
			// cannot rotate
			return;
		}
	}

	// rotate and update tile
	rotationStatus = newRotation;
	copyArray4OfVec2(tile, allRotationsShapes[tileType][newRotation]);
	updateTile();
}
// Move the tile to the left for 1 grid, when left is pressed
void moveTileToLeft() {
	vec2 direction = vec2(-1, 0);
	if (moveTile(direction))
	{
		updateTile();
	}

}
// Move the tile to the right for 1 grid, when right is pressed
void moveTileToRight() {
	vec2 direction = vec2(1, 0);
	if (moveTile(direction))
	{
		updateTile();
	}
}




//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'a':
			increaseTheta_Arm();
			break;
		case 'd':
			decreaseTheta_Arm();
			break;
		case 'w':
			increasePhi_Arm();
			break;
		case 's':
			decreasePhi_Arm();
			break;
		case 'p':// pause the game
			paused = !paused;
			break;
		case 'r': // 'r' key restarts the game
			restartGame();
			break;
	}
	glutPostRedisplay();
}

// increase or decrease theta_arm
void increaseTheta_Arm() {
	theta_arm += 1;
	drawArm();

	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 36*4*sizeof(vec4), armPoints, GL_DYNAMIC_DRAW);
	glBindVertexArray(vboIDs[6]);
}

void decreaseTheta_Arm() {
	theta_arm -= 1;
	drawArm();

	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 36*4*sizeof(vec4), armPoints, GL_DYNAMIC_DRAW);
	glBindVertexArray(vboIDs[6]);
}

void increasePhi_Arm() {
	phi_arm += 1;
	drawArm();

	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 36*4*sizeof(vec4), armPoints, GL_DYNAMIC_DRAW);
	glBindVertexArray(vboIDs[6]);
}

void decreasePhi_Arm() {
	phi_arm -= 1;
	drawArm();

	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 36*4*sizeof(vec4), armPoints, GL_DYNAMIC_DRAW);
	glBindVertexArray(vboIDs[6]);
}


// Restart game, when 'r' is pressed
void restartGame() {

	cout << "restarting game" << endl;

	// empty the board
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 20; ++j) {
			board[i][j] = false;
		}
	}

	// set all board colors to black
	for (int i = 0; i < 1200 * 6; ++i)
	{
		boardcolours[i] = black;
	}
	updateVboOfBoardColor();

	// create new tile

	newTile();
	displaytile();



	// resets line counters


	// reset titmer
	paused = false;
	halted = false;

	// increment gameround counter
	gameRound++;

	// reset speed
	timerIntervial = 1000;

	glutTimerFunc(timerIntervial, Timer, gameRound);

}


//-------------------------------------------------------------------------------------------------------------------
// Handles timer callback

void Timer(int value) {

	if (value != gameRound)
	{
		// not this round's timer
		return;
	}

	if (!(halted || paused))
	{
		cout << "shit's going man" << endl;
		moveTileDownAndSettle();
	}

	glutTimerFunc(timerIntervial, Timer, gameRound);
}


// Move tile down by 1
// check whether it should be settled
// update board

void moveTileDownAndSettle() {
	cout << 111111 << endl;
	if(moveTileDown()) {
		// update tile 
		updateTile();
	} else {
		// if cannot move tile dowm
		// settleTile
		settleTile();

		// generate new tile
		newTile();

		// check whether game over
		if (isGameOver())
		{
			// do nothing
			halted = true;

			// prompt game is over
			cout << "Game Over" << endl
				<< "press 'r' to restart" << endl
				<< "press 'q' to quit" << endl 
				<< endl;
			return;
		}

		// display new tile
		displaytile();
	}
}

// move tile down by 1 grid
bool moveTileDown() {
	vec2 direction = vec2(0,-1);
	return moveTile(direction);
}




//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{

	srand(time(0));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(timerIntervial, Timer, gameRound);	/* timer interval 1s */
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loopf
	return 0;
}