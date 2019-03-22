#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <GL/glut.h>
#include "Matrix.h"
#include "WaveFrontOBJ.h"

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

unsigned floorTexID;

int frame = 0;
int width, height;
int oldX, oldY;

int dis_x, dis_y;
double dis_drag;
double trans_dis_x = 0;
double trans_dis_y = 0;
double trans_dis_z = 0;

void cow_translate();
void cow_rotate();

/*******************************************************************/
//(PA #2) : ������ �ذ��ϱ� ���� ������ �߰������� �����Ͻʽÿ�.
/*******************************************************************/

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
		float* c = cameras[i];											// 'c' points the coordinate of i-th camera.
		wld2cam.push_back(Matrix());								// Insert {0} matrix to wld2cam vector.
		glPushMatrix();													// Push the current matrix of GL into stack.
		
		glLoadIdentity();												// Set the GL matrix Identity matrix.
		gluLookAt(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8]);		// Setting the coordinate of camera.
		glGetFloatv(GL_MODELVIEW_MATRIX, wld2cam[i].matrix());			// Read the world-to-camera matrix computed by gluLookAt.
		cam2wld.push_back(wld2cam[i].inverse());						// Get the camera-to-world matrix.
		
		glPopMatrix();													// Transfer the matrix that was pushed the stack to GL.
		
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
			
			glutSolidSphere(1.0, 10, 10);
			drawAxis(5);													// Draw x, y, and z axis.
			
			glPopMatrix();													// Call the matrix on stack. wld2cam[cameraIndex].matrix() in here.
		}
	}
}





/*********************************************************************************
* Draw 'cow' object.
**********************************************************************************/
void InitCow(){
	cow = new WaveFrontOBJ("cow.obj");

	// Make display list. After this, you can draw cow using 'cowID'.
	glPushMatrix();						// Push the current matrix of GL into stack.
	glLoadIdentity();					// Set the GL matrix Identity matrix.
	glTranslatef(0, -cow->bbmin.pos.y, -8);	// Set the location of cow.
	glRotatef(-90, 0, 1, 0);			// Set the direction of cow. These information are stored in the matrix of GL.
	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());	// Read the modelview matrix about location and direction set above, and store it in cow2wld matrix.
	glPopMatrix();						// Pop the matrix on stack to GL.

}





void drawCow()
{
	glPushMatrix();		// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing cow.

	// The information about location of cow to be drawn is stored in cow2wld matrix.
	// If you change the value of the cow2wld matrix or the current matrix, cow would rotate or move.
	glMultMatrixf(cow2wld.matrix());

	cow->Draw();						// Draw the cow on display list.
	drawAxis(5);						// Draw x, y, and z axis.

	glPopMatrix();			// Pop the matrix in stack to GL. Change it the matrix before drawing cow.
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

	glColor3d(0.35, 0.2, 0.1);

	// Draw background rectangle.
	glBegin(GL_POLYGON);
	glVertex3f(2000, -0.2, 2000);
	glVertex3f(2000, -0.2, -2000);
	glVertex3f(-2000, -0.2, -2000);
	glVertex3f(-2000, -0.2, 2000);
	glEnd();


	// Set color of the floor.
	// Assign checker-patterned texture.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, floorTexID);

	// Draw the floor. Match the texture's coordinates and the floor's coordinates resp. 
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	glVertex3d(-12, -0.1, -12);		// Texture's (0,0) is bound to (-12,-0.1,-12).
	glTexCoord2d(1, 0);
	glVertex3d(12, -0.1, -12);		// Texture's (1,0) is bound to (12,-0.1,-12).
	glTexCoord2d(1, 1);
	glVertex3d(12, -0.1, 12);		// Texture's (1,1) is bound to (12,-0.1,12).
	glTexCoord2d(0, 1);
	glVertex3d(-12, -0.1, 12);		// Texture's (0,1) is bound to (-12,-0.1,12).
	glEnd();

	glDisable(GL_TEXTURE_2D);
	
	drawAxis(5);				// Draw x, y, and z axis.
}





/*********************************************************************************
* Call this part whenever display events are needed.
* Display events are called in case of re-rendering by OS. ex) screen movement, screen maximization, etc.
* Or, user can occur the events by using glutPostRedisplay() function directly.
* this part is called in main() function by registering on glutDisplayFunc(display).
**********************************************************************************/
void display()
{
	glClearColor(0, 0.6, 0.8, 1);									// Clear color setting

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear the screen
	
	drawCamera();													// and draw all of them.
	drawFloor();													// Draw floor.
	
	drawCow();														// Draw cow.

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
	glShadeModel(GL_FLAT);			 // Set Flat Shading
	glEnable(GL_DEPTH_TEST);         // Enables Depth Testing

	// Define lighting for the scene
	glEnable(GL_LIGHTING);
	float light_pos[] = { 1.0, 1.0, 1.0, 0.0 };
	float light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	float light_diffuse[] = { 0.9, 0.9, 0.9, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);

	InitCamera();
	InitCow();
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
	//(PA #2) :�߰����� �ڵ� �ۼ��� �ʿ��մϴ�.
	//���콺�� �̵� �Ÿ��� ���� ������Ʈ�� �����̵� �Ÿ��� �����Ͻʽÿ�.
	/*******************************************************************/

	dis_x = x - oldX;
	dis_y = y - oldY;
	dis_drag = sqrt( dis_x * dis_x + dis_y * dis_y)

	oldX = x;
	oldY = y;

	cow_translate();

	glutPostRedisplay();
}





/*********************************************************************************
* Call this part whenever user types keyboard.
* This part is called in main() function by registering on glutKeyboardFunc(onKeyPress).
**********************************************************************************/
void onKeyPress(unsigned char key, int x, int y)
{
	//ī�޶� ���� ����
	if ((key >= '1') && (key <= '5'))
		cameraIndex = key - '1';

	/*******************************************************************/
	//(PA #2) :�߰����� �ڵ� �ۼ��� �ʿ��մϴ�.
	//������Ʈ�� ȸ�� ���뿩�ο� �𵨸� ��ǥ���� ���� �����ϱ� ���� �Լ��� �ۼ��Ͻʽÿ�.
	/*******************************************************************/

	switch(key) {
		case: 'x':
		case: 'X':
			trans_dis_x = drag_x;
			trans_dis_y = 0;
			trans_dis_z = 0;
			break;
		case: 'y':
		case: 'Y':
			break;
		case: 'z':
		case: 'Z':
			break;
		case: 'r':
		case: 'R':
			break;

		default:
			break;

	}

	glutPostRedisplay();
}





void cow_rotate(){
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixf(cow2wld.matrix()); //load current transformation matrix of object

	/*******************************************************************/
	//(PA #2) :�߰����� �ڵ� �ۼ��� �ʿ��մϴ�.
	//���� ���õ� �࿡ ���Ͽ� ������Ʈ�� ȸ���� �����Ͻʽÿ�.
	/*******************************************************************/

	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
	glPopMatrix();
}





void cow_translate(){
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixf(cow2wld.matrix()); //load current transformation matrix of object
		glTranslatef(trans_dis_x,trans_dis_y,trans_dis_z);
	/*******************************************************************/
	//(PA #2) :�߰����� �ڵ� �ۼ��� �ʿ��մϴ�.
	//���� ���õ� �࿡ ���Ͽ� ������Ʈ�� �����̵��� �����Ͻʽÿ�.
	/*******************************************************************/

	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
	glPopMatrix();

	trans_dis_x += trans_dis_x;
	trans_dis_y += trans_dis_y;
	trans_dis_z += trans_dis_z;
}





void idle() {

	/*******************************************************************/
	//(PA #2) :�߰����� �ڵ� �ۼ��� �ʿ��մϴ�.
	//�߰����� �Է��� ���� ���� �������� �����Ͻʽÿ�.
	/*******************************************************************/

	glutPostRedisplay();
}





//------------------------------------------------------------------------------
void main(int argc, char* argv[])
{
	width = 800;
	height = 600;
	frame = 0;
	glutInit(&argc, argv);							// Initialize openGL.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// Initialize display mode. This project will use float buffer and RGB color.
	glutInitWindowSize(width, height);				// Initialize window size.
	glutInitWindowPosition(100, 100);				// Initialize window coordinate.
	glutCreateWindow("PA2");
	
	glutDisplayFunc(display);						// Register display function to call that when drawing screen event is needed.
	glutReshapeFunc(reshape);						// Register reshape function to call that when size of the window is changed.
	glutKeyboardFunc(onKeyPress);					// Register onKeyPress function to call that when user presses the keyboard.
	glutMouseFunc(onMouseButton);					// Register onMouseButton function to call that when user moves mouse.
	glutMotionFunc(onMouseDrag);					// Register onMouseDrag function to call that when user drags mouse.

	glutIdleFunc(idle);

	initialize();									// Initialize the other thing.
	glutMainLoop();									// Execute the loop which handles events.
}