#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <GL/glut.h>
#include "Matrix.h"
#include "WaveFrontOBJ.h"

#define ROTATED true
#define _COW 1
#define _BUNNY 2

// 'cameras' stores infomation of 5 cameras.
float cameras[5][9] =
{
	{ 28, 18, 28, 0, 2, 0, 0, 1, 0 },
	{ 28, 18, -28, 0, 2, 0, 0, 1, 0 },
	{ -28, 18, 28, 0, 2, 0, 0, 1, 0 },
	{ -12, 12, 0, 0, 2, 0, 0, 1, 0 },
	{ 0, 100, 0, 0, 0, 0, 1, 0, 0 }
};
int cameraCount = sizeof(cameras) / sizeof(cameras[0]);

int cameraIndex;
vector<Matrix> wld2cam, cam2wld;

// Variables for 'cow' object.
Matrix cow2wld;
WaveFrontOBJ* cow;

// Variables for 'bunny' object.
Matrix bunny2wld;
WaveFrontOBJ* bunny;

unsigned floorTexID;

int frame = 0;
int width, height;
int oldX, oldY;

void translate();
void rotate();

int selectMode;
bool picking = false;

/*******************************************************************/
//(PA #2,#3) ºˆ«‡«— ≥ªøÎ √ﬂ∞°
/*******************************************************************/
bool checkRotation = !ROTATED;
char input_axis = 'x';
float drag_move = 0.0f;
char select_space = 'm';
int pick_object = _COW;
float drag_X_move = 0.0f;
float drag_Y_move = 0.0f;

char select_vector = ' ';
char light_mode = ' ';

bool shademodel = true;

//------------------------------------------------------------------------------
void munge(int x, float& r, float& g, float& b)
{
	r = (x & 255) / float(255);
	g = ((x >> 8) & 255) / float(255);
	b = ((x >> 16) & 255) / float(255);
}





//------------------------------------------------------------------------------
int unmunge(float r, float g, float b)
{
	return (int(r) + (int(g) << 8) + (int(b) << 16));
}





/*********************************************************************************
* Draw x, y, z axis of current frame on screen.
* x, y, and z are corresponded Red, Green, and Blue, resp.
**********************************************************************************/
void drawAxis(float len)
{
	glDisable(GL_LIGHTING);		// Lighting is not needed for drawing axis.
	glBegin(GL_LINES);			// Start drawing lines.
	glColor3d(1, 0, 0);			// color of x-axis is red.
	glVertex3d(0, 0, 0);
	glVertex3d(len, 0, 0);		// Draw line(x-axis) from (0,0,0) to (len, 0, 0). 
	glColor3d(0, 1, 0);			// color of y-axis is green.
	glVertex3d(0, 0, 0);
	glVertex3d(0, len, 0);		// Draw line(y-axis) from (0,0,0) to (0, len, 0).
	glColor3d(0, 0, 1);			// color of z-axis is  blue.
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, len);		// Draw line(z-axis) from (0,0,0) - (0, 0, len).
	glEnd();					// End drawing lines.
	glEnable(GL_LIGHTING);
}





void InitCamera(){

	// initialize camera frame transforms.
	for (int i = 0; i < cameraCount; i++)
	{
		float* c = cameras[i];													// 'c' points the coordinate of i-th camera.
		wld2cam.push_back(Matrix());											// Insert {0} matrix to wld2cam vector.
		glPushMatrix();															// Push the current matrix of GL into stack.

		glLoadIdentity();														// Set the GL matrix Identity matrix.
		gluLookAt(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8]);		// Setting the coordinate of camera.
		glGetFloatv(GL_MODELVIEW_MATRIX, wld2cam[i].matrix());					// Read the world-to-camera matrix computed by gluLookAt.
		cam2wld.push_back(wld2cam[i].inverse());								// Get the camera-to-world matrix.

		glPopMatrix();															// Transfer the matrix that was pushed the stack to GL.
	}
	cameraIndex = 0;
}





void drawCamera()
{
	int i;
	// set viewing transformation.
	glLoadMatrixf(wld2cam[cameraIndex].matrix());

	// draw cameras.
	for (i = 0; i < (int)wld2cam.size(); i++)
	{
		if (i != cameraIndex)
		{
			glPushMatrix();													// Push the current matrix on GL to stack. The matrix is wld2cam[cameraIndex].matrix().
			glMultMatrixf(cam2wld[i].matrix());								// Multiply the matrix to draw i-th camera.
			if (selectMode == 0)											// selectMode == 1 means backbuffer mode.
			{
				drawAxis(5);												// Draw x, y, and z axis.
			}
			else
			{
				float r, g, b;
				glDisable(GL_LIGHTING);										// Disable lighting in backbuffer mode.
				munge(i + 1, r, g, b);										// Match the corresponding (i+1)th color to r, g, b. You can change the color of camera on backbuffer.
				glColor3f(r, g, b);											// Set r, g, b the color of camera.
			}

			glutSolidSphere(1.0, 10, 10);

			glPopMatrix();													// Call the matrix on stack. wld2cam[cameraIndex].matrix() in here.
		}
	}
}





/*********************************************************************************
* Draw 'cow' object.
**********************************************************************************/
void InitCow(){
	cow = new WaveFrontOBJ("cow_pa4.obj");

	glPushMatrix();											// Push the current matrix of GL into stack.
	glLoadIdentity();										// Set the GL matrix Identity matrix.
	glTranslatef(0, -cow->bbmin.pos.y, -8);					// Set the location of cow.
	glRotatef(-90, 0, 1, 0);								// Set the direction of cow. These information are stored in the matrix of GL.
	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());		// Read the modelview matrix about location and direction set above, and store it in cow2wld matrix.
	glPopMatrix();											// Pop the matrix on stack to GL.

}





void drawCow()
{
	glPushMatrix();											// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing cow.

	// The information about location of cow to be drawn is stored in cow2wld matrix.
	// If you change the value of the cow2wld matrix or the current matrix, cow would rotate or move.
	glMultMatrixf(cow2wld.matrix());

	if (selectMode == 0)									// selectMode == 1 means backbuffer mode.
	{
		drawAxis(5);										// Draw x, y, and z axis.
	}
	else
	{
		float r, g, b;
		glDisable(GL_LIGHTING);								// Disable lighting in backbuffer mode.
		munge(32, r, g, b);									// Match the corresponding constant color to r, g, b. You can change the color of camera on backbuffer
		glColor3f(r, g, b);
	}
	
	cow->Draw();											
	/*******************************************************************/
	//(PA #4) : cow object¿« normal¿ª ±◊∏Æ¥¬ «‘ºˆ∏¶ √ﬂ∞°«œΩ Ω√ø¿.
	/*******************************************************************/
  
  	// f 누르면 Face normal vector 그림
    if ( select_vector == 'f') {
        cow->Draw_FN();
    // n 누르면 vertex normal vector 그림    
    } else if (select_vector == 'n') {
        cow->Draw_VN();
    }
    
//    std::cout << "cow normal:";
//    std::for_each(cow->faceNormals.begin(), cow->faceNormals.end(), [](Vector i)->void {std::cout << "("<< i.x<<","<< i.y<<","<< i.z <<")"<<std::endl;});
//    std::cout << '\n';
    

	glPopMatrix();											// Pop the matrix in stack to GL. Change it the matrix before drawing cow.
}





void Initbunny(){
	bunny = new WaveFrontOBJ("bunny_pa4.obj");

	glPushMatrix();											// Push the current matrix of GL into stack.
	glLoadIdentity();										// Set the GL matrix Identity matrix.
	glTranslatef(0, -bunny->bbmin.pos.y, 8);					// Set the location of bunny.
	glRotatef(180, 0, 1, 0);								// Set the direction of bunny. These information are stored in the matrix of GL.
	glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());		// Read the modelview matrix about location and direction set above, and store it in bunny2wld matrix.
	glPopMatrix();											// Pop the matrix on stack to GL.

}





void drawbunny()
{
	glPushMatrix();		// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing bunny.

	// The information about location of bunny to be drawn is stored in bunny2wld matrix.
	// If you change the value of the bunny2wld matrix or the current matrix, bunny would rotate or move.
	glMultMatrixf(bunny2wld.matrix());

	if (selectMode == 0)									// selectMode == 1 means backbuffer mode.
	{
		drawAxis(8);										// Draw x, y, and z axis.
	}
	else
	{
		float r, g, b;
		glDisable(GL_LIGHTING);								// Disable lighting in backbuffer mode.
		munge(33, r, g, b);									// Match the corresponding constant color to r, g, b. You can change the color of camera on backbuffer
		glColor3f(r, g, b);
	}
	
	bunny->Draw();											// Draw the bunny 
	/*******************************************************************/
	//(PA #4) : bunny object¿« normal¿ª ±◊∏Æ¥¬ «‘ºˆ∏¶ √ﬂ∞°«œΩ Ω√ø¿.
	/*******************************************************************/
    if ( select_vector == 'f') {
        bunny->Draw_FN();
    } else if (select_vector == 'n') {
        bunny->Draw_VN();
    }

	glPopMatrix();											// Pop the matrix in stack to GL. Change it the matrix before drawing bunny.
}





/*********************************************************************************
* Draw floor on 3D plane.
**********************************************************************************/
void drawFloor()
{
	if (frame == 0)
	{
		// Initialization part.
		// After making checker-patterned texture, use this repetitively.

		// Insert color into checker[] according to checker pattern.
		const int size = 8;
		unsigned char checker[size*size * 3];
		for (int i = 0; i < size*size; i++)
		{
			if (((i / size) ^ i) & 1)
			{
				checker[3 * i + 0] = 100;
				checker[3 * i + 1] = 100;
				checker[3 * i + 2] = 100;
			}
			else
			{
				checker[3 * i + 0] = 200;
				checker[3 * i + 1] = 200;
				checker[3 * i + 2] = 200;
			}
		}

		// Make texture which is accessible through floorTexID. 
		glGenTextures(1, &floorTexID);
		glBindTexture(GL_TEXTURE_2D, floorTexID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, checker);
	}

	glDisable(GL_LIGHTING);

	// Set background color.
	if (selectMode == 0)
		glColor3d(0.35, 0.2, 0.1);
	else
	{
		// In backbuffer mode.
		float r, g, b;
		munge(34, r, g, b);
		glColor3f(r, g, b);
	}

	// Draw background rectangle.
	glBegin(GL_POLYGON);
	glVertex3f(-2000, -0.2, -2000);
	glVertex3f(-2000, -0.2, 2000);
	glVertex3f(2000, -0.2, 2000);
	glVertex3f(2000, -0.2, -2000);
	glEnd();


	// Set color of the floor.
	if (selectMode == 0)
	{
		// Assign checker-patterned texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorTexID);
	}
	else
	{
		// Assign color on backbuffer mode.
		float r, g, b;
		munge(35, r, g, b);
		glColor3f(r, g, b);
	}

	// Draw the floor. Match the texture's coordinates and the floor's coordinates resp. 
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	glVertex3d(-12, -0.1, -12);		// Texture's (0,0) is bound to (-12,-0.1,-12).
	glTexCoord2d(0, 1);
	glVertex3d(-12, -0.1, 12);		// Texture's (0,1) is bound to (-12,-0.1,12).
	glTexCoord2d(1, 1);
	glVertex3d(12, -0.1, 12);		// Texture's (1,1) is bound to (12,-0.1,12).
	glTexCoord2d(1, 0);
	glVertex3d(12, -0.1, -12);		// Texture's (1,0) is bound to (12,-0.1,-12).

	glEnd();

	if (selectMode == 0)
	{
		glDisable(GL_TEXTURE_2D);
		drawAxis(5);				// Draw x, y, and z axis.
	}
}





void Lighting()
{
	/*******************************************************************/
	//(PA #4) : ¥ŸæÁ«— ±§ø¯¿ª ±∏«ˆ«œΩ Ω√ø¿.
	//1. Point light / Directional light / Spotlight∏¶ º≠∑Œ ¥Ÿ∏• ªˆ¿∏∑Œ ±∏«ˆ«œΩ Ω√ø¿.
	//2. ±§ø¯¿« ¿ßƒ°∏¶ ±∏(sphere)∑Œ «•«ˆ«œΩ Ω√ø¿.
	//3. Directional light / Spotlight¿« ∞ÊøÏ ∫˚¿« ¡¯«‡πÊ«‚¿ª º±∫–¿∏∑Œ «•«ˆ«œΩ Ω√ø¿.
	/*******************************************************************/

    glDisable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    
    // point light
    float light_pos[] = {0.0f, 11.0f, 1.0f, 1.0f};
    float light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float light_diffuse[] = {0.5f, 1.0f, 0.0f, 1.0f};
    float light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT1, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    
    // directional light
    float light_pos1[] = {11.0f, 11.0f, 11.0f, 0.0f};
    float light_ambient1[] = {0.5f, 0.5f, 0.5f, 1.0f};
    float light_diffuse1[] = {1.0f, 0.0f, 0.0f, 1.0f};
    float light_specular1[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT2, GL_POSITION, light_pos1);
    glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient1);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse1);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular1);
    
    
    
    // spot light
    float light_pos2[] = {-11.0f, 11.0f, -11.0f, 0.0f};
    float light_dir2[] = { 5.0f, -5.0f, 5.0f};
    float light_ambient2[] = {0.4f, 0.4f, 0.4f, 1.0f};
    float light_diffuse2[] = {0.0f, 0.3f, 0.8f, 1.0f};
    float light_specular2[] = {0.8f, 0.8f, 0.8f, 1.0f};
    
    glLightfv(GL_LIGHT3, GL_POSITION, light_pos2);
    glLightfv(GL_LIGHT3, GL_AMBIENT, light_ambient2);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, light_diffuse2);
    glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular2);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, light_dir2);
    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 15.0f);
    glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 20.0f);
    
    
    // material : gold
    float noMat[] = { 0.0f, 0.0f, 0.0f, 1.0f};
    float matAmb[] {0.25f, 0.2f, 0.07f, 1.0f};
    float matDif[] = {0.75f, 0.6f, 0.22f, 1.0f};
    float matSpec[] = {0.62f, 0.55f, 0.36f, 1.0f};
    float matShineness = 51.2f;
    
    glMaterialfv(GL_FRONT, GL_EMISSION , noMat);
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT, GL_SHININESS , matShineness);
    
    glDisable(GL_LIGHTING);
    
    // light mode에 따라 광원 그
    if (light_mode == 'p') {
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
            glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
            glutSolidSphere(0.5, 100, 100);
        glPopMatrix();
    }
    
    else if (light_mode == 'd') {
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
//            glMaterialfv(GL_FRONT, GL_EMISSION, Emission);
            glTranslatef(light_pos1[0], light_pos1[1], light_pos1[2]);
            glutSolidSphere(0.5, 100, 100);
        glPopMatrix();
        
        glBegin(GL_LINES);
            glVertex3f(light_pos1[0], light_pos1[1], light_pos1[2]);
            glVertex3f(0.0f, 0.0f, 0.0f);
        
        glEnd();
    }
    
    else if ( light_mode == 's') {
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
//            glMaterialfv(GL_FRONT, GL_EMISSION, Emission);
            glTranslatef(light_pos2[0], light_pos2[1], light_pos2[2]);
            glutSolidSphere(0.5, 100, 100);
        glPopMatrix();
    
        glBegin(GL_LINES);
            glVertex3f(light_pos2[0], light_pos2[1], light_pos2[2]);
            glVertex3f(light_pos2[0]+light_dir2[0], light_pos2[1]+light_dir2[1], light_pos2[2]+light_dir2[2]);
        glEnd();
    } else {
        glEnable(GL_LIGHT0);
    }
    
}





/*********************************************************************************
* Call this part whenever display events are needed.
* Display events are called in case of re-rendering by OS. ex) screen movement, screen maximization, etc.
* Or, user can occur the events by using glutPostRedisplay() function directly.
* this part is called in main() function by registering on glutDisplayFunc(display).
**********************************************************************************/
void display()
{
	if (selectMode == 0)
		glClearColor(0, 0.6, 0.8, 1);								// Clear color setting
	else
		glClearColor(0, 0, 0, 1);									// When the backbuffer mode, clear color is set to black

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear the screen

	if (shademodel)
		glShadeModel(GL_FLAT);										// Set Flat Shading
	else
		glShadeModel(GL_SMOOTH);

	drawCamera();													// and draw all of them.
	drawFloor();													// Draw floor.

	drawCow();														// Draw cow.
	drawbunny();

	Lighting();

	glFlush();

	// If it is not backbuffer mode, swap the screen. In backbuffer mode, this is not necessary because it is not presented on screen.
	if (selectMode == 0)
		glutSwapBuffers();

	frame++;
}





/*********************************************************************************
* Call this part whenever size of the window is changed.
* This part is called in main() function by registering on glutReshapeFunc(reshape).
**********************************************************************************/
void reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);            // Select The Projection Matrix
	glLoadIdentity();                       // Reset The Projection Matrix
	float aspect = width / float(height);
	gluPerspective(45, aspect, 1, 1024);

	glMatrixMode(GL_MODELVIEW);             // Select The Modelview Matrix
	glLoadIdentity();                       // Reset The Projection Matrix
}





//------------------------------------------------------------------------------
void initialize()
{
	// Set up OpenGL state
	glEnable(GL_DEPTH_TEST);         // Enables Depth Testing
	// Initialize the matrix stacks
	reshape(width, height);
	// Define lighting for the scene
	float light_pos[] = { 1.0, 1.0, 1.0, 0.0 };
	float light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	float light_diffuse[] = { 0.9, 0.9, 0.9, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);

	InitCamera();
	InitCow();
	Initbunny();
}





/*********************************************************************************
* Call this part whenever mouse button is clicked.
* This part is called in main() function by registering on glutMouseFunc(onMouseButton).
**********************************************************************************/
void onMouseButton(int button, int state, int x, int y)
{
	y = height - y - 1;
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			printf("Left mouse click at (%d, %d)\n", x, y);

			// After drawing object on backbuffer, you can recognize which object is selected by reading pixel of (x, y).
			// Change the value of selectMode to 1, then draw the object on backbuffer when display() function is called. 
			selectMode = 1;
			display();
			glReadBuffer(GL_BACK);
			unsigned char pixel[3];
			glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
			int pixel_value = unmunge(pixel[0], pixel[1], pixel[2]);
			printf("pixel = %d\n", pixel_value);

			if (picking) {
				/*******************************************************************/
				//(PA #2,#3) ºˆ«‡«— ≥ªøÎ √ﬂ∞°
				/*******************************************************************/
                switch (pixel_value) {
                    case 32:        // pick cow
                        pick_object = _COW;
                        break;
                    case 33:        // pick bunny
                        pick_object = _BUNNY;
                        break;
                    default:
                        pick_object = 0;
                        break;
                }
                picking = false;
			}

			selectMode = 0;
			// Save current clicked location of mouse here, and then use this on onMouseDrag function. 
			oldX = x;
			oldY = y;
		}
	}

	glutPostRedisplay();
}





/*********************************************************************************
* Call this part whenever user drags mouse.
* Input parameters x, y are coordinate of mouse on dragging.
* Value of global variables oldX, oldY is stored on onMouseButton,
* Then, those are used to verify value of x - oldX,  y - oldY to know its movement.
**********************************************************************************/
void onMouseDrag(int x, int y)
{
	y = height - y - 1;
	printf("in drag (%d, %d)\n", x - oldX, y - oldY);
	
	/*******************************************************************/
	//(PA #2,#3)  ºˆ«‡«— ≥ªøÎ √ﬂ∞°
	/*******************************************************************/
    drag_move = (x - oldX + y - oldY)/10;
    drag_X_move = x - oldX;
    drag_Y_move = y - oldY;
    
    if (select_space == 'v' && checkRotation) {
        rotate();
    }
    else {
        translate();
    }

	oldX = x;
	oldY = y;
	glutPostRedisplay();
}





/*********************************************************************************
* Call this part whenever user types keyboard.
* This part is called in main() function by registering on glutKeyboardFunc(onKeyPress).
**********************************************************************************/
void onKeyPress(unsigned char key, int x, int y)
{
	//ƒ´∏ﬁ∂Û Ω√¡° ∫Ø∞Ê
	if ((key >= '1') && (key <= '5'))
		cameraIndex = key - '1';

	/*******************************************************************/
	//(PA #2,#3)  ºˆ«‡«— ≥ªøÎ √ﬂ∞°
	/*******************************************************************/
    switch (key) {
        case 'X':
        case 'x':
            input_axis = key;
            break;
        case 'Y':
        case 'y':
            input_axis = key;
            break;
        case 'z':
        case 'Z':
            input_axis = key;
            break;
        case 'R':
        case 'r':
            printf("rotate");
            checkRotation = !checkRotation;
            if (checkRotation) {
                rotate();
            }
            break;
        case 'p':
        case 'P':
            printf("picking");
            picking = true;
            break;
        case 'm':
        case 'M':
            printf("modeling space");
            select_space = 'm';
            break;
        case 'V':
        case 'v':
            printf("Viewing space");
            select_space = 'v';
            break;
        case 'F':
        case 'f':
            if (select_vector != 'f') {
                select_vector = 'f';
            } else {
                select_vector= ' ';
            }
            break;
            
        case 'N':
        case 'n':
            if (select_vector != 'n') {
                select_vector = 'n';
            } else {
                select_vector= ' ';
            }
            break;
            
        default:
            break;
    }

	if (key == 's') {
		shademodel = !shademodel;
		cow->isFlat = shademodel;
		bunny->isFlat = shademodel;
	}

	glutPostRedisplay();
}






void SpecialKey(int key, int x, int y)
{
	/*******************************************************************/
	//(PA #4) : F1 / F2 / F3 πˆ∆∞ø° µ˚∂Û º≠∑Œ ¥Ÿ∏• ±§ø¯¿Ã On/Off µ«µµ∑œ ±∏«ˆ«œΩ Ω√ø¿.
	/*******************************************************************/

	// f1, f2, f3 를 누를 때마다 light mode 변
    switch (key) {
            //directional light
        case GLUT_KEY_F1:
            if (light_mode != 'd') {
                light_mode = 'd';
                glEnable(GL_LIGHT2);
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHT1);
                glDisable(GL_LIGHT3);
            } else {
                light_mode = ' ';
                glEnable(GL_LIGHT0);
                glDisable(GL_LIGHT1);
                glDisable(GL_LIGHT2);
                glDisable(GL_LIGHT3);
            }
            break;
            
            // point light
        case GLUT_KEY_F2:
            if ( light_mode != 'p') {
                light_mode = 'p';
                glEnable(GL_LIGHT1);
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHT2);
                glDisable(GL_LIGHT3);
            } else {
                light_mode = ' ';
                glEnable(GL_LIGHT0);
                glDisable(GL_LIGHT1);
                glDisable(GL_LIGHT2);
                glDisable(GL_LIGHT3);
            }
            break;
            // spot light
        case GLUT_KEY_F3:
            if ( light_mode != 's') {
                light_mode = 's';
                glEnable(GL_LIGHT3);
                glDisable(GL_LIGHT0);
                glDisable(GL_LIGHT1);
                glDisable(GL_LIGHT2);
            } else {
                light_mode = ' ';
                glEnable(GL_LIGHT0);
                glDisable(GL_LIGHT1);
                glDisable(GL_LIGHT2);
                glDisable(GL_LIGHT3);
            }
            break;
            
        default:
            light_mode = ' ';
            glEnable(GL_LIGHT0);
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT2);
            glDisable(GL_LIGHT3);
            break;
    }
	glutPostRedisplay();
}

void rotateOnModelingSpace() {
    if (pick_object == _COW) {
        glMultMatrixf(cow2wld.matrix());
    } else if (pick_object == _BUNNY) {
        glMultMatrixf(bunny2wld.matrix());
    }
    else return;
    
    switch (input_axis)
    {
        case 'X' :
        case 'x' :
            glRotatef(3.6f, 1.0f, 0.0f, 0.0f);
            break;
        case 'Y' :
        case 'y' :
            glRotatef(3.6f,0.0f,1.0f, 0.0f);
            break;
        case 'Z':
        case 'z':
            glRotatef(3.6f,0.0f, 0.0f,1.0f);
            break;
        default:
            break;
    }
    
    if (pick_object == _COW) {
        glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
    } else if (pick_object == _BUNNY) {
        glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
    }
    
}

void rotateOnViewingSpace() {
    
    if (pick_object == _COW) {
        glMultMatrixf(cow2wld.matrix());
    } else if (pick_object == _BUNNY) {
        glMultMatrixf(bunny2wld.matrix());
    }
    
    glMultMatrixf(cam2wld[cameraIndex].matrix());
    glTranslatef(wld2cam[cameraIndex].matrix()[12], wld2cam[cameraIndex].matrix()[13], wld2cam[cameraIndex].matrix()[14]);
    
    switch (input_axis)
    {
        case 'X' :
        case 'x' :
        case 'Y' :
        case 'y' :
        case 'Z':
        case 'z':
        default:
            glRotatef(drag_move*10, 1.0f, 0.0f, 0.0f);
            break;
    }
    glTranslatef(-wld2cam[cameraIndex].matrix()[12], -wld2cam[cameraIndex].matrix()[13], -wld2cam[cameraIndex].matrix()[14]);
    
    glMultMatrixf(wld2cam[cameraIndex].matrix());
    
    if (pick_object == _COW) {
        glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
    } else if (pick_object == _BUNNY) {
        glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
    }
}




void rotate(){
	glPushMatrix();
	glLoadIdentity();

	/*******************************************************************/
	//(PA #2,#3)  ºˆ«‡«— ≥ªøÎ √ﬂ∞°
	/*******************************************************************/
    
    if (select_space == 'm') {
        printf("modeling space");
        rotateOnModelingSpace();
    }
    else if ( select_space == 'v') {
        printf("viewing space");
        rotateOnViewingSpace();
    }

	glPopMatrix();
}

void translateOnModelingSpace() {
    if (pick_object == _COW) {
        glMultMatrixf(cow2wld.matrix());
    } else if (pick_object == _BUNNY) {
        glMultMatrixf(bunny2wld.matrix());
    }
    else return;
    
    switch (input_axis)
    {
        case 'X':
        case 'x':
            glTranslatef(drag_move, 0.0f, 0.0f);
            break;
        case 'Y':
        case 'y':
            glTranslatef(0.0f, drag_move, 0.0f);
            break;
        case 'Z':
        case 'z':
            glTranslatef(0.0f, 0.0f,drag_move);
            break;
        default:
            break;
    }
    if (pick_object == _COW) {
        glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
    } else if (pick_object == _BUNNY) {
        glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
    }
}

void translateOnViewingSpace() {
    //    glMultMatrixf(wld2cam[cameraIndex].matrix());
    glMultMatrixf(cam2wld[cameraIndex].matrix());
    switch (input_axis)
    {
        case 'X':
        case 'x':
        case 'Y':
        case 'y':
            glTranslatef(drag_X_move/10, drag_Y_move/10, 0.0f);
            break;
        case 'Z':
        case 'z':
            glTranslatef(0.0f, 0.0f,drag_move);
            break;
        default:
            break;
    }
    glMultMatrixf(wld2cam[cameraIndex].matrix());
    
    if (pick_object == _COW) {
        glMultMatrixf(cow2wld.matrix());
        glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
    } else if (pick_object == _BUNNY) {
        glMultMatrixf(bunny2wld.matrix());
        glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
    }
    
}



void translate(){
	glPushMatrix();
	glLoadIdentity();

	/*******************************************************************/
	//(PA #2,#3)  ºˆ«‡«— ≥ªøÎ √ﬂ∞°
	/*******************************************************************/
    
    if (select_space == 'm') {
        printf("modeling space");
        translateOnModelingSpace();
    }
    else if ( select_space == 'v') {
        printf("viewing space");
        translateOnViewingSpace();
    }


	glPopMatrix();
}





void idle() {

	/*******************************************************************/
	//(PA #2,#3)  ºˆ«‡«— ≥ªøÎ √ﬂ∞°
	/*******************************************************************/
    if (select_space == 'm' && checkRotation) {
        rotate();
    }

	glutPostRedisplay();
}






//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	width = 800;
	height = 600;
	frame = 0;
	glutInit(&argc, argv);							// Initialize openGL.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// Initialize display mode. This project will use float buffer and RGB color.
	glutInitWindowSize(width, height);				// Initialize window size.
	glutInitWindowPosition(100, 100);				// Initialize window coordinate.
	glutCreateWindow("PA4");

	glutDisplayFunc(display);						// Register display function to call that when drawing screen event is needed.
	glutReshapeFunc(reshape);						// Register reshape function to call that when size of the window is changed.
	glutKeyboardFunc(onKeyPress);					// Register onKeyPress function to call that when user presses the keyboard.
	glutMouseFunc(onMouseButton);					// Register onMouseButton function to call that when user moves mouse.
	glutMotionFunc(onMouseDrag);					// Register onMouseDrag function to call that when user drags mouse.

	glutIdleFunc(idle);

	glutSpecialFunc(SpecialKey);

	initialize();									// Initialize the other thing.
	glutMainLoop();									// Execute the loop which handles events.
    
    return 0;
}
