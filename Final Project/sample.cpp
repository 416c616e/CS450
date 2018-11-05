#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <random>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Alan Neads

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.


// title of these windows:

const char *WINDOWTITLE = { "Final Project - Mazes -- Alan Neads" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	"Red",
	"Yellow",
	"Green",
	"Cyan",
	"Blue",
	"Magenta",
	"White",
	"Black"
};


// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// lighting stuff:

// utility to create an array from a multiplier and an array:
float *
Array3(float a, float b, float c)
{
	static float array[4];
	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}
float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}
void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}
float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrt(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

float White[] = { 1.,1.,1.,1. };
float Black[] = { 0.2, 0.2, 0.2, 1.0 };
float *
MulArray3(float factor, float array0[3])
{
	static float array[4];
	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

void
SetMaterial(float r, float g, float b, float shininess)
{
	glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4f, White));
	glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., White));
	glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
	glMaterialf(GL_BACK, GL_SHININESS, 2.f);
	glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_FRONT, GL_AMBIENT, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.8f, White));
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void
SetPointLight(int ilight, float x, float y, float z, float r, float g, float b)
{
	glLightfv(ilight, GL_POSITION, Array3(x, y, z));
	glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	glLightf(ilight, GL_LINEAR_ATTENUATION, 0.5);
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
	glEnable(ilight);
}

void
SetSpotLight(int ilight, float x, float y, float z, float xdir, float ydir, float zdir, float r, float g, float b)
{
	glLightfv(ilight, GL_POSITION, Array3(x, y, z));
	glLightfv(ilight, GL_SPOT_DIRECTION, Array3(xdir, ydir, zdir));
	glLightf(ilight, GL_SPOT_EXPONENT, 1.);
	glLightf(ilight, GL_SPOT_CUTOFF, 20.);
	glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.1);
	glEnable(ilight);
}


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };

// maze parameters:
// maze source: https://github.com/corporateshark/random-maze-generator/blob/master/Maze.cpp
const int NumCells = 25;

unsigned char* g_Maze = new unsigned char[NumCells* NumCells];

// current traversing position
int g_PtX;
int g_PtY;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(0, NumCells - 1);
std::uniform_int_distribution<> dis4(0, 3);

enum eDirection
{
	eDirection_Invalid = 0,
	eDirection_Up = 1,
	eDirection_Right = 2,
	eDirection_Down = 4,
	eDirection_Left = 8
};


int playerX;
int playerY;
eDirection playerFacingDirection = eDirection_Up;

//                   0  1  2  3  4  5  6  7  8
//                      U  R     D           L
int Heading_X[9] = { 0, 0,+1, 0, 0, 0, 0, 0,-1 };
int Heading_Y[9] = { 0,-1, 0, 0,+1, 0, 0, 0, 0 };
int Mask[9] = {
	0,
	eDirection_Down | eDirection_Down << 4,
	eDirection_Left | eDirection_Left << 4,
	0,
	eDirection_Up | eDirection_Up << 4,
	0,
	0,
	0,
	eDirection_Right | eDirection_Right << 4
};


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to use the z-buffer
GLuint	BoxList;				// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void	Axes( float );
void	HsvRgb( float[3], float [3] );

// maze functions:
void InitMaze();
int CellIdx();
int RandomInt();
int RandomInt4();
bool IsDirValid(eDirection);
eDirection GetDirection();
void GenerateMaze();
void playerMove(char);


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// generate maze

	InitMaze( );


	// setup all the graphics stuff:

	InitGraphics( );


	// create the display structures that will not change:

	InitLists( );


	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );


	// setup all the user interface stuff:

	InitMenus( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if( DepthBufferOn != 0 )
		glEnable( GL_DEPTH_TEST );
	else
		glDisable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	glViewport( 0, 0,  vx, vy );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );

	gluPerspective( 90., ((float)vx)/((float)vy),	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	// set the eye position, look-at position, and up-vector:
	float translatedPlayerX = -NumCells/2.0f + playerX;
	float translatedPlayerY = NumCells/2.0f + playerY;
	float faceX = 0;
	float faceY = 0;

	if (playerFacingDirection == eDirection_Up) {
		faceX += 1;
	} else if (playerFacingDirection == eDirection_Left) {
		faceY -= 1;
	} else if (playerFacingDirection == eDirection_Down) {
		faceX -= 1;
	} else if (playerFacingDirection == eDirection_Right) {
		faceY += 1;
	}

	if (WhichProjection == ORTHO){
		gluLookAt(translatedPlayerY - NumCells, 3 + Scale, translatedPlayerX + NumCells, translatedPlayerY - NumCells, 0, translatedPlayerX + NumCells, 1., 0., 0.);
	} else {
		gluLookAt(translatedPlayerY - NumCells, 0.5, translatedPlayerX + NumCells,		 translatedPlayerY - NumCells + faceX, 0.5, translatedPlayerX + NumCells + faceY, 0., 1., 0.);
	}

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );


	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);


	// draw the current object:
	float fulldx = 0.5;
	float fulldz = 0.5;
	float dx = 0.4999;
	float dz = 0.4999;
	glPushMatrix();
		glTranslatef(-NumCells/2.0f, 0, NumCells/2.0f);
		for (int y = 0; y < NumCells; y++) {
			glPushMatrix();
				glTranslatef( y, 0, 0 );
				for (int x = 0; x < NumCells; x++) {
					char v = g_Maze[y * NumCells + x];

					glPushMatrix();
						glTranslatef(0, 0, x);
						glBegin(GL_QUADS);
							if (!(v & eDirection_Up)) { // really -x
								SetMaterial(0.5,0,0,1);
								glColor3f(0.5, 0., 0.);
								glNormal3f(1., 0., 0.);

								glVertex3f(-dx, 0, fulldz);
								glVertex3f(-dx, 1, fulldz);
								glVertex3f(-dx, 1, -fulldz);
								glVertex3f(-dx, 0, -fulldz);
							}
							if (!(v & eDirection_Down)) { // really +x
								SetMaterial(1, 0, 0, 1);
								glColor3f(1., 0., 0.);
								glNormal3f(-1., 0., 0.);

								glVertex3f(dx, 0, fulldz);
								glVertex3f(dx, 0, -fulldz);
								glVertex3f(dx, 1, -fulldz);
								glVertex3f(dx, 1, fulldz);
							}
							if (!(v & eDirection_Right)) { // really +z
								SetMaterial(0, 0, 1, 1);
								glColor3f(0., 0., 1.);
								glNormal3f(0., 0., -1.);

								glVertex3f(-fulldx, 0, dz);
								glVertex3f(fulldx, 0, dz);
								glVertex3f(fulldx, 1, dz);
								glVertex3f(-fulldx, 1, dz);
							}
							if (!(v & eDirection_Left)) { // really -z
								SetMaterial(0, 0, 0.5, 1);
								glColor3f(0., 0., 0.5);
								glNormal3f(0., 0., 1.);

								glVertex3f(-fulldx, 0, -dz);
								glVertex3f(-fulldx, 1, -dz);
								glVertex3f(fulldx, 1, -dz);
								glVertex3f(fulldx, 0, -dz);
							}
							if ((x == NumCells - 1 && y == NumCells - 1) || (x == 0 && y == 0)) {
								SetMaterial(0.8, 0.49, 0.19, 1);
								glColor3f(0.8, 0.49, 0.19);
							} else {
								SetMaterial(0, 1, 0, 1);
								glColor3f(0., 1., 0.);
							}
							glNormal3f(0., 1., 0.);

							glVertex3f(-0.5, 0,  0.5);
							glVertex3f( 0.5, 0,  0.5);
							glVertex3f( 0.5, 0, -0.5);
							glVertex3f(-0.5, 0, -0.5);
						glEnd();

						if (WhichProjection == ORTHO && x == playerX && y == playerY) {
							glPushMatrix();
							SetMaterial(0, 1.0, 1.0, 1);
							glColor3f(0.0, 1.0, 1.0);
							glScalef(0.8,1.0,0.8);
							glutSolidCube(1.0);
							glPopMatrix();
						}
					glPopMatrix();
				}
			glPopMatrix();
		}
	glPopMatrix();

	glDisable(GL_LIGHTING);

	glEnable(GL_LIGHT0);
	SetSpotLight(GL_LIGHT0, translatedPlayerY - NumCells, 0.2, translatedPlayerX + NumCells, faceX, 0.0, faceY, 1.0, 1.0, 0.0);
	
	gen.seed(2);
	for (int i = 0; i < 6; i++) {
		glEnable(GL_LIGHT1 + i);
		SetPointLight(GL_LIGHT1 + i, -NumCells / 2.0f + RandomInt(), 0.5, NumCells / 2.0f + RandomInt(), 0.2, 0.2, 0.2);
	}

	glEnable(GL_LIGHT7);
	SetPointLight(GL_LIGHT7, -NumCells / 2.0f + NumCells - 1, 0.5, NumCells / 2.0f + NumCells - 1, 0.5, 0.5, 0.5);

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Overhead",  ORTHO );
	glutAddMenuEntry( "First Person",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "View Position",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case 'W':
			playerMove('W');
			break;
		case 'w':
			playerMove('w');
			break;
		case 's':
		case 'S':
			playerMove('s');
			break;
		case 'a':
		case 'A':
			playerMove('a');
			break;
		case 'd':
		case 'D':
			playerMove('d');
			break;
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r, g, b;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}
void InitMaze()
{
	gen.seed(time(NULL));
	//gen.seed(1);
	std::fill(g_Maze, g_Maze + NumCells * NumCells, 0);
	g_PtX = RandomInt();
	g_PtY = RandomInt();
	GenerateMaze();
}

// return the current index in g_Maze
int CellIdx()
{
	return g_PtX + NumCells * g_PtY;
}

int RandomInt()
{
	return static_cast<int>(dis(gen));
}

int RandomInt4()
{
	return static_cast<int>(dis4(gen));
}

bool IsDirValid(eDirection Dir)
{
	int NewX = g_PtX + Heading_X[Dir];
	int NewY = g_PtY + Heading_Y[Dir];

	if (!Dir || NewX < 0 || NewY < 0 || NewX >= NumCells || NewY >= NumCells) return false;

	return !g_Maze[NewX + NumCells * NewY];
}

eDirection GetDirection()
{
	eDirection Dir = eDirection(1 << RandomInt4());

	while (true)
	{
		for (int x = 0; x < 4; x++)
		{
			if (IsDirValid(Dir)) { return eDirection(Dir); }

			Dir = eDirection(Dir << 1);

			if (Dir > eDirection_Left) { Dir = eDirection_Up; }
		}

		Dir = eDirection((g_Maze[CellIdx()] & 0xf0) >> 4);

		// nowhere to go
		if (!Dir) return eDirection_Invalid;

		g_PtX += Heading_X[Dir];
		g_PtY += Heading_Y[Dir];

		Dir = eDirection(1 << RandomInt4());
	}
}

void GenerateMaze()
{
	int Cells = 0;

	for (eDirection Dir = GetDirection(); Dir != eDirection_Invalid; Dir = GetDirection())
	{
		g_Maze[CellIdx()] |= Dir;

		g_PtX += Heading_X[Dir];
		g_PtY += Heading_Y[Dir];

		g_Maze[CellIdx()] = Mask[Dir];
	}

	playerX = 0;
	playerY = 0;
}

void playerMove(char key)
{
	if (key == 'w') {
		int offsetX = playerX;
		int offsetY = playerY;

		int targetX = 0;
		int targetY = 0;

		if (playerFacingDirection == eDirection_Up) {
			targetY += 1;
		} else if (playerFacingDirection == eDirection_Down) {
			targetY -= 1;
		} else if (playerFacingDirection == eDirection_Left) {
			targetX -= 1;
		} else if (playerFacingDirection == eDirection_Right) {
			targetX += 1;
		}

		if (offsetX + targetX < 0 || offsetX + targetX >= NumCells) {
			return;
		}
		if (offsetY + targetY < 0 || offsetY + targetY >= NumCells) {
			return;
		}

		char v = g_Maze[offsetY * NumCells + offsetX];

		int up = v & eDirection_Up;
		int down = v & eDirection_Down;
		int left = v & eDirection_Left;
		int right = v & eDirection_Right;

		if (left && targetX == -1) {
			playerX += targetX;
		} else if (right && targetX == 1) {
			playerX += targetX;
		} else if (up && targetY == -1) {
			playerY += targetY;
		} else if (down && targetY == 1) {
			playerY += targetY;
		}
	} else if (key == 'W') {
		int offsetX = playerX;
		int offsetY = playerY;

		int targetX = 0;
		int targetY = 0;

		if (playerFacingDirection == eDirection_Up) {
			targetY += 1;
		}
		else if (playerFacingDirection == eDirection_Down) {
			targetY -= 1;
		}
		else if (playerFacingDirection == eDirection_Left) {
			targetX -= 1;
		}
		else if (playerFacingDirection == eDirection_Right) {
			targetX += 1;
		}

		if (offsetX + targetX < 0 || offsetX + targetX >= NumCells) {
			return;
		}
		if (offsetY + targetY < 0 || offsetY + targetY >= NumCells) {
			return;
		}

		char v = g_Maze[offsetY * NumCells + offsetX];

		int up = v & eDirection_Up;
		int down = v & eDirection_Down;
		int left = v & eDirection_Left;
		int right = v & eDirection_Right;

		if (targetX == -1) {
			playerX += targetX;
		}
		else if (targetX == 1) {
			playerX += targetX;
		}
		else if (targetY == -1) {
			playerY += targetY;
		}
		else if (targetY == 1) {
			playerY += targetY;
		}
	} else if (key == 's') {
		int offsetX = playerX;
		int offsetY = playerY;

		int targetX = 0;
		int targetY = 0;

		if (playerFacingDirection == eDirection_Up) {
			targetY -= 1;
		} else if (playerFacingDirection == eDirection_Down) {
			targetY += 1;
		} else if (playerFacingDirection == eDirection_Left) {
			targetX += 1;
		} else if (playerFacingDirection == eDirection_Right) {
			targetX -= 1;
		}

		if (offsetX + targetX < 0 || offsetX + targetX >= NumCells) {
			return;
		}
		if (offsetY + targetY < 0 || offsetY + targetY >= NumCells) {
			return;
		}

		char v = g_Maze[offsetY * NumCells + offsetX];

		int up = v & eDirection_Up;
		int down = v & eDirection_Down;
		int left = v & eDirection_Left;
		int right = v & eDirection_Right;

		if (left && targetX == -1) {
			playerX += targetX;
		} else if (right && targetX == 1) {
			playerX += targetX;
		} else if (up && targetY == -1) {
			playerY += targetY;
		} else if (down && targetY == 1) {
			playerY += targetY;
		}
	}  else if (key == 's') {
		int offsetX = playerX;
		int offsetY = playerY;

		int targetX = 0;
		int targetY = 0;

		if (playerFacingDirection == eDirection_Up) {
			targetY -= 1;
		} else if (playerFacingDirection == eDirection_Down) {
			targetY += 1;
		} else if (playerFacingDirection == eDirection_Left) {
			targetX += 1;
		} else if (playerFacingDirection == eDirection_Right) {
			targetX -= 1;
		}

		if (offsetX + targetX < 0 || offsetX + targetX >= NumCells) {
			return;
		}
		if (offsetY + targetY < 0 || offsetY + targetY >= NumCells) {
			return;
		}

		char v = g_Maze[offsetY * NumCells + offsetX];

		int up = v & eDirection_Up;
		int down = v & eDirection_Down;
		int left = v & eDirection_Left;
		int right = v & eDirection_Right;

		if (left && targetX == -1) {
			playerX += targetX;
		} else if (right && targetX == 1) {
			playerX += targetX;
		} else if (up && targetY == -1) {
			playerY += targetY;
		} else if (down && targetY == 1) {
			playerY += targetY;
		}
	} else if (key == 'a') {
		if (playerFacingDirection == eDirection_Up) {
			playerFacingDirection = eDirection_Left;
		} else if (playerFacingDirection == eDirection_Down) {
			playerFacingDirection = eDirection_Right;
		} else if (playerFacingDirection == eDirection_Left) {
			playerFacingDirection = eDirection_Down;
		} else if (playerFacingDirection == eDirection_Right) {
			playerFacingDirection = eDirection_Up;
		}
	} else if (key == 'd') {
		if (playerFacingDirection == eDirection_Up) {
			playerFacingDirection = eDirection_Right;
		} else if (playerFacingDirection == eDirection_Down) {
			playerFacingDirection = eDirection_Left;
		} else if (playerFacingDirection == eDirection_Left) {
			playerFacingDirection = eDirection_Up;
		} else if (playerFacingDirection == eDirection_Right) {
			playerFacingDirection = eDirection_Down;
		}
	}
}
