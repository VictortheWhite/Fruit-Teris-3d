/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <cstdlib>

using namespace std;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current title
vec2 title[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the title, on the grid
vec2 titlePos = vec2(5, 19); // The position of the current title using grid coordinates ((0,0) is the bottom left corner)

// the type and rotation of the current title
int titleType;
int rotationStatus;

// An array storing all possible orientations of all possible titles
// The 'title' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] = {
	{vec2(0,0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
		{vec2(0,0), vec2(0,-1), vec2(0, 1), vec2(1, -1)},
		{vec2(0,0), vec2(1, 0), vec2(-1,0), vec2(1,  1)},
		{vec2(0,0), vec2(0, 1), vec2(0,-1), vec2(-1, 1)}
	/*
	{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1)},     
	{vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0)},  
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}
	*/
};

/*
vec2 allRotationsShapes[4][4][4] = {
	{
		{vec2(0,0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
		{vec2(0,0), vec2(0,-1), vec2(0, 1), vec2(1, -1)},
		{vec2(0,0), vec2(1, 0), vec2(-1,0), vec2(1,  1)},
		{vec2(0,0), vec2(0, 1), vec2(0,-1), vec2(-1, 1)}
	},
	{
	
	},
	{
	
	},
	{
	
	}
}
*/

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

// furit colors of current title
vec4 currentTitleFruitColors[4];

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As titles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// fuc decalrations

// for sepcial keys(arrow keys)
void rotateTitle();
void accelerateTitle();
void restartGame();

void moveTitleToLeft();
void moveTitleToRight();
bool moveTitleDown();

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
// title operations

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
bool collide(vec2* Title, vec2 direction) {
	return occupied(titlePos.x + Title[0].x + direction.x, titlePos.y + Title[0].y + direction.y)
		|| occupied(titlePos.x + Title[1].x + direction.x, titlePos.y + Title[1].y + direction.y)
		|| occupied(titlePos.x + Title[2].x + direction.x, titlePos.y + Title[2].y + direction.y)
		|| occupied(titlePos.x + Title[3].x + direction.x, titlePos.y + Title[3].y + direction.y);
}

// Given (x,y), tries to move the title x squares to the right and y squares down
// Returns true if the title was successfully moved, or false if there was some issue
bool moveTitle(vec2 direction) {
	// if not collide, move title
	if(!collide(title, direction)) {
		/*
		titlePos.x += direction.x;
		titlePos.y += direction.y;
		*/
		titlePos += direction;
		return true;
	}
	// otherwise return false
	return false;
}

// Places the current title
void settleTitle()
{
	// update the board vertex colour VBO
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			// each square has 6 vertex
			int vIndex = 6 * (10 * (titlePos.y + title[i].y) + (titlePos.x + title[i].x)) + j;
			boardcolours[vIndex] = currentTitleFruitColors[i];
		}
	}

	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glBindVertexArray(0);
	glBindVertexArray(vboIDs[3]);

	// update occupied cells array
	for (int i = 0; i < 4; ++i)
	{
		int x = titlePos.x + title[i].x;
		int y = titlePos.y + title[i].y;
		board[x][y] = true;
	}
}

// When the current title is moved or rotated (or created), update the VBO containing its vertex position data
void updateTitle() {
	// Bind the VBO containing current title vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the title,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = titlePos.x + title[i].x; 
		GLfloat y = titlePos.y + title[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

// whether a title is out of upper bound
// used when generating position of new title
bool isTitleOutOfUpperBound() {
	for (int i = 0; i < 4; ++i)
	{
		if (titlePos.y + title[i].y > 19)
		{
			return true;
		}
	}

	return false;
}

bool isTitleOutOfLeftBound() {
	for (int i = 0; i < 4; ++i)
	{
		if (titlePos.x + title[i].x < 0)
		{
			return true;
		}
	}

	return false;
}

bool isTitleOutOfRightBound() {
	for (int i = 0; i < 4; ++i)
	{
		if (titlePos.x + title[i].x > 9)
		{
			return true;
		}
	}

	return false;
}

// Called at the start of play and every time a title is placed
void newTitle()
{

	// for now, set the title type to be 0
	// and rotation to be 0
	titleType = 0;
	rotationStatus = 0;

	int titleXPos = randomNum(10);
	int titleRotation = randomNum(4);

	printf("aaaaaaa %d\n",titleRotation);

	titlePos = vec2(titleXPos , 19); // Put the title at the top of the board

	// Update the geometry VBO of current title
	for (int i = 0; i < 4; i++) {
		title[i] = allRotationsLshape[titleRotation][i]; // Get the 4 pieces of the new title
	}

	// adjust position of title to fit in the screen
	if (isTitleOutOfUpperBound())
	{
		titlePos.y += -1;
	}
	if (isTitleOutOfRightBound())
	{
		titlePos.x += -1;
		if (isTitleOutOfRightBound())
		{
			titlePos.x += -1;
		}
	}
	if (isTitleOutOfLeftBound())
	{
		titlePos.x += 1;
		if (isTitleOutOfLeftBound())
		{
			titlePos.x += 1;
		}
	}


	updateTitle(); 


	// generate random color for each fruit
	for (int i = 0; i < 4; i++)
	{
		currentTitleFruitColors[i] = fruitColors[randomNum(5)];
	}

}

void displayTitle() {
	// Update the color VBO of current title
	vec4 newcolours[24];

	for (int i = 0; i < 24; i++) {
		newcolours[i] = currentTitleFruitColors[i/6]; 
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current title vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
// game logic

// return true if game is over
bool isGameOver() {
	return collide(title, vec2(0,0));
}

// set full row's eliminated to be true
// return true if exists full row to be eliminated
bool checkFullRow(bool **eliminated) {
	int flag = false;
	for (int j = 0; j < 20; ++j)
	{
		bool fullRow = true;
		for (int i = 0; i < 10; ++i)
		{
			if (!board[i][j])
			{
				fullRow = false;
			}
		}
		// if full row, set eliminated to be true
		if (fullRow)
		{
			flag = true;
			for (int i = 0; i < 10; ++i)
			{
				eliminated[i][j] = true;
			}
		}
	}

	return flag;
}

// check a grid's down, right-hand, northeast and sourtheast side
// return true if fruits to be eliminated

bool checkSingleGrid(int x, int y, bool **eliminated) {

	bool flag = false;
	vec4 thisGridColor = getGridColor(x, y);
	if (isVec4Equal(thisGridColor, black))
	{
		return false;;
	}

	// check down side
	if (y >= 2)
	{
		int count = 1;
		
		while(isVec4Equal(thisGridColor, getGridColor(x, y - count))) {
			count++;
			if (y-count < 0)
			{
				break;
			}
		}

		if (count >= 3)
		{
			flag = true;
			for (int i = 0; i < count; ++i)
			{
				eliminated[x][y-i] = true;
			}
		}
	}

	
	// check right-hand side
	if(x < 8) {
		int count = 1;
		while(isVec4Equal(thisGridColor, getGridColor(x+count, y))) {
			count++;
			if (x+count > 9)
			{
				break;
			}
		}

		if (count >= 3)
		{
			flag = true;
			for (int i = 0; i < count; ++i)
			{
				eliminated[x+i][y] = true;
			}
		}

	}
	
	// check north-east side
	if (y < 18 && x < 8)
	{
		int count = 1;
		while(isVec4Equal(thisGridColor, getGridColor(x+count, y+count))) {
			count++;
			if (x+count> 9 || y+count > 19)
			{
				break;
			}
		}

		if (count >= 3)
		{
			flag = true;
			for (int i = 0; i < count; ++i)
			{
				eliminated[x+i][y+i] = true;
			}
		}
	}

	// check south-east side
	if (y >= 2 && x < 8)
	{
		int count = 1;
		while(isVec4Equal(thisGridColor, getGridColor(x+count, y-count))) {
			count++;
			if (x+count > 9 || y-count < 0)
			{
				break;
			}
		}

		if (count >= 3)
		{
			flag = true;
			for (int i = 0; i < count; ++i)
			{
				eliminated[x+i][y-i] = true;
			}
		}
	}

	return flag;
	

}

// check all grids for 4 consecutive smae fruits
// return true if there are fruits to be eliminated
bool checkEliminatedFruit(bool **eliminated) {
	bool flag = false;
	for (int j = 20; j >= 0; --j)
	{
		for (int i = 0; i < 10; ++i)
		{
			if(checkSingleGrid(i, j, eliminated))
				flag = true;
		}
	}

	return flag;

}

void updateBoradAfterEliminating(bool **eliminated) {
	for (int i = 0; i < 10; ++i)
	{
		int count = 0;	// the distance to be shift down
		for (int j = 0; j < 19; ++j)
		{
			if (eliminated[i][j])
			{
				count++;
			} else {
				// move grid down by count
				// update occupied board
				board[i][j-count] = board[i][j];
				// update color
				vec4 newColor = getGridColor(i, j);
				for (int k = 0; k < 6; ++k)
				{
					int vertexIndex = 6 * ( 10 * ( j - count ) + i) + k;
					boardcolours[vertexIndex] = newColor;
				}
			}
		}
	}

}

void updateVBOAfterEliminating() {
	// update vbo
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glBindVertexArray(0);
	glBindVertexArray(vboIDs[3]);
}

void eliminate(bool &needsUpdate) {
	bool **eliminated = new bool*[10];	// true for grid to be eliminated

	for (int i = 0; i < 10; ++i)
	{
		eliminated[i] = new bool[20];
		for (int j = 0; j < 20; ++j)
		{
			eliminated[i][j] = false;
		}
	}

	// check full rows
	bool fullRow = checkFullRow(eliminated);

	// check consecutive same fruits
	bool consecutiveFruits = checkEliminatedFruit(eliminated);

	// if somethins is eliminated
	// recursively call eliminate untill no fruit can be eliminated
	if (fullRow || consecutiveFruits)
	{
		needsUpdate = true;
		updateBoradAfterEliminating(eliminated);
		eliminate(needsUpdate);
	}

	for (int i = 0; i < 10; ++i)
	{
		delete[] eliminated[i];
	}

	delete[] eliminated;
}

void eliminateBoardAndUpdate() {
	bool needUpdate = false;

	eliminate(needUpdate);

	cout << "needsUpdate: " << needUpdate << endl;

	if (needUpdate)
	{
		updateVBOAfterEliminating();
	}
}

//-------------------------------------------------------------------------------------------------------------------
// game initialization
void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++) {
		boardcolours[i] = black; // Let the empty cells on the board be black
	}

	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
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
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current title initially
void initCurrentTitle()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current title vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current title vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
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

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current title
	initGrid();
	initBoard();
	initCurrentTitle();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newTitle(); // create new next title
	displayTitle();

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current title (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current title (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


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
	switch(key) {
		case GLUT_KEY_UP: rotateTitle(); break;	// rotate title if "up"
		case GLUT_KEY_LEFT: moveTitleToLeft(); break;
		case GLUT_KEY_RIGHT: moveTitleToRight(); break;
		case GLUT_KEY_DOWN: accelerateTitle(); break;
		default: break;
	}
}

// special key actions

// Rotates the current title, when up is pressed
void rotateTitle() {      
	// new rotation
	int newRotation = rotationStatus;

	newRotation = (newRotation + 1) % 4;

	// check whether possible to rotate
	if(collide(allRotationsLshape[newRotation], vec2(0,0))) {
		// if the rotated title will collide
		if(!collide(allRotationsLshape[newRotation], vec2(1,0))) {
			// if move to the right by 1 will work
			titlePos.x += 1;
		} else if(!collide(allRotationsLshape[newRotation], vec2(-1,0))) {
			// if move to the left by 1 will work
			titlePos.x += -1;
		} else if(!collide(allRotationsLshape[newRotation], vec2(2, 0))) {
			// if move to the right by 2 will work
			titlePos.x += 2;
		} else if(!collide(allRotationsLshape[newRotation], vec2(-2, 0))) {
			// if move to the left by 2 will work
			titlePos.x += -2;
		} else {
			// cannot rotate
			return;
		}
	}

	// rotate and update title
	rotationStatus = newRotation;
	copyArray4OfVec2(title, allRotationsLshape[newRotation]);
	updateTitle();
}
// Move the title to the left for 1 grid, when left is pressed
void moveTitleToLeft() {
	vec2 direction = vec2(-1, 0);
	if (moveTitle(direction))
	{
		updateTitle();
	}

}
// Move the title to the right for 1 grid, when right is pressed
void moveTitleToRight() {
	vec2 direction = vec2(1, 0);
	if (moveTitle(direction))
	{
		updateTitle();
	}
}
// Accelerate the falling title, when down is pressed
void accelerateTitle() {
	// let it to fall for one for now,,,
	if(moveTitleDown()) {
		printf("title moved down\n");
		// update title 
		updateTitle();
	} else {
		// if cannot move title dowm
		// settleTitle
		printf("settleTitle\n");
		settleTitle();
		// check full row
		newTitle();
		displayTitle();
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
		case 'r': // 'r' key restarts the game
			restartGame();
			break;
	}
	glutPostRedisplay();
}

// Restart game, when 'r' is pressed
void restartGame() {

	printf("restarting game\n");
	exit(0);
	// empty the board

	// create new title

	// resets line counters

}


//-------------------------------------------------------------------------------------------------------------------
// Handles timer callback

void Timer(int value) {

	if(moveTitleDown()) {
		printf("title moved down\n");
		// update title 
		updateTitle();
	} else {
		// if cannot move title dowm
		// settleTitle
		printf("settleTitle\n");
		settleTitle();

		// try to eliminate
		eliminateBoardAndUpdate();

		// generate new title
		newTitle();

		// check whether game over
		if (isGameOver())
		{
			restartGame();
			return;
		}

		// display new title
		displayTitle();
	}

	glutTimerFunc(1000, Timer, 0);
}

// move title down by 1 grid
bool moveTitleDown() {
	vec2 direction = vec2(0,-1);
	return moveTitle(direction);
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{

	cout << (vec4(1,2,3,4)==vec4(1,2,3,4)) << endl << endl << endl;

	srand(time(0));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
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
	glutTimerFunc(1000, Timer, 0);	/* timer interval 1s */
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
