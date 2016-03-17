//#define WIN32

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <climits>
#include <GL/glut.h>
#include <sys/stat.h>

#define VIEWING_DISTANCE_MIN  3.0
using namespace std;

//Function delclarations
void parseObjFile(FILE* input);

//Vectors for storing vertices
std::vector<GLfloat*> vertices;
std::vector<GLint*> faces;

float g_rotation = 0;
float g_rotation_speed = 0.2f;

static GLfloat g_fViewDistance =  VIEWING_DISTANCE_MIN;
static int g_yClick = 0;

GLfloat scale_factor = 1.0;
string display_type = "";

//File IO vars
FILE* objectFile = NULL;
string filename;

//Object center
int objCenter[] = {0,0,0};

//Vars for tracking the xyz min/max
long maxx = LONG_MIN;
long maxy = LONG_MIN;
long maxz = LONG_MIN;
long minx = LONG_MAX;
long miny = LONG_MAX;
long minz = LONG_MAX;

//Position of mouse
int oldX;
int oldY;

//Distance mouse has moved in x and y
int vdist;
int hdist;

//State of mouse click 
bool LeftIsPressed = false;
bool RightIsPressed = false;

//User, object transform vars
GLfloat zoom_factor = 1;
static GLfloat moveCords[] = {0.0,0.0,0.0};

//Vertical rotation
int hrotate = 0;
int vrotate = 0;

//Camera data
GLfloat aspect;
GLfloat field_of_view_angle = 90;

//Light 1 settings
GLfloat diffuse0[]={1.0, 0.0, 0.0, 1.0};
GLfloat ambient0[]={1.0, 0.0, 0.0, 1.0};
GLfloat specular0[]={1.0, 0.0, 0.0, 1.0}; 
GLfloat light0_pos[]={1.0, 2.0, 3,0, 1.0}; 
//Light 2 settings
GLfloat diffuse1[]={1.0, 0.0, 0.0, 1.0};
GLfloat ambient1[]={1.0, 0.0, 0.0, 1.0};
GLfloat specular1[]={1.0, 0.0, 0.0, 1.0}; 
GLfloat light1_pos[]={-1.0, 2.0, 3,0, 1.0}; 

GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat diffuse[] = {1.0, 0.8, 0.0, 1.0};
GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};

//Window info
struct glutWindow{
	int width;
	int height;
	string title;
	//float field_of_view_angle;
	GLdouble z_near;
	GLdouble z_far;
};
glutWindow win;

void drawPoints(){
	glBegin(GL_POINTS);
		for (int i = 0; i<vertices.size(); i++)
		{
			glVertex3fv(vertices[i]);
		}
	glEnd();
}

void drawLines(){
	glBegin(GL_LINES);  // GL_LINE_STRIP
		for (int i = 0; i<faces.size(); i++)
		{
			glVertex3fv(vertices[faces[i][0] - 1]);
			glVertex3fv(vertices[faces[i][1] - 1]);
			glVertex3fv(vertices[faces[i][1] - 1]);
			glVertex3fv(vertices[faces[i][2] - 1]);
			glVertex3fv(vertices[faces[i][2] - 1]);
			glVertex3fv(vertices[faces[i][0] - 1]);
		}
	glEnd();
}

void drawPolygons(){
	glBegin(GL_TRIANGLES);
		for (int i = 0; i<faces.size(); i++)
		{
			glNormal3fv(vnormals[faces[i][0] - 1]);
			glVertex3fv(vertices[faces[i][0] - 1]);
			glNormal3fv(vnormals[faces[i][1] - 1]);
			glVertex3fv(vertices[faces[i][1] - 1]);
			glNormal3fv(vnormals[faces[i][2] - 1]);
			glVertex3fv(vertices[faces[i][2] - 1]);

			//Calc normal vectors
			GLfloat v1_x = vertices[faces[i][1] - 1][0] - vertices[faces[i][0] - 1][0];
			GLfloat v1_y = vertices[faces[i][1] - 1][1] - vertices[faces[i][0] - 1][1];
			GLfloat v1_z = vertices[faces[i][1] - 1][2] - vertices[faces[i][0] - 1][2]; 

			GLfloat v2_x = vertices[faces[i][2] - 1][0] - vertices[faces[i][0] - 1][0];
			GLfloat v2_y = vertices[faces[i][2] - 1][1] - vertices[faces[i][0] - 1][1];
			GLfloat v2_z = vertices[faces[i][2] - 1][2] - vertices[faces[i][0] - 1][2]; 
			//cout << "v1:"<<v1_x<<","<<v1_y<<","<<v1_z<<endl<<i<<endl;
			//(y1z2 - z1y2)x + (z1x2 - x1z2)y + (x1y2 - y1x2)z
			GLfloat vf_x = (v1_y * v2_z - v1_z * v2_y);
			GLfloat vf_y = (v1_z * v2_x - v1_x * v2_z);
			GLfloat vf_z = (v1_x * v2_y - v1_y * v2_x);

			//Calc mag of vect
			GLfloat v = sqrt(pow(vf_x,2) + pow(vf_y,2) + pow(vf_z,2));

			GLfloat normal[] = {vf_x/v,vf_y/v,vf_z/v}; 

			//cout << "n:"<<normal[0]<< "," <<normal[1]<<","<<normal[2]<<endl;

		}
	glEnd();
}

void parseObjFile(FILE* input){
	faces.clear();
	vertices.clear();
	char c;
	GLfloat f1, f2, f3, *arrayfloat;
	GLint d1, d2, d3, *arrayint;
	
	//Read to end of input and find min/max for each component
	while (!feof(input)){
		fscanf(input, "%c", &c);
		if (c == 'v') {
			arrayfloat = new GLfloat[3];
			fscanf(input, "%f %f %f", &f1, &f2, &f3);
			
			if(f1>maxx){
				maxx = f1;
			}
			if(f2>maxy){
				maxy=f2;
			}
			if(f3>maxz){
				maxz=f3;
			}
			if(f1<minx){
				minx = f1;
			}
			if(f2<miny){
				miny=f2;
			}
			if(f3<minz){
				minz=f3;
			}
			arrayfloat[0] = f1;
			arrayfloat[1] = f2;
			arrayfloat[2] = f3;
			vertices.push_back(arrayfloat);
		}
		else if (c == 'f'){
				arrayint = new GLint[3];
				fscanf(input, "%d %d %d", &d1, &d2, &d3);
				arrayint[0] = d1;
				arrayint[1] = d2;
				arrayint[2] = d3;
				faces.push_back(arrayint);
			}
	}
	//Find the max length
	GLfloat lenx = maxx-minx;
	GLfloat leny = maxy-miny;
	GLfloat lenz = maxz-minz;

	if(lenx > leny){
		if(lenx > lenz){
			scale_factor = lenx;
		}
	}else if(leny > lenz){
		scale_factor = leny;
	}else{
		scale_factor = lenz;
	}
	
	//cout << "\nThe max x:" << maxx << endl <<"The max y:"<< maxy << endl << "The max z:" << maxz << endl;
	//cout << "\nThe min x:" << minx << endl <<"The min y:"<< miny << endl << "The min z:" << minz << endl;
	fclose(input);
}

GLfloat objCent_x = 0;
GLfloat objCent_y = 0;
GLfloat objCent_z = 0;

GLdouble bottom = -20;
GLdouble top = 20;
GLdouble leftCorner = -5.0f;
GLdouble rightCorner = -5.0f;

void display(void){
	glClearColor(0,0,0,0.0); //Set clear color 	to blue

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear color buffer and depth buffer
	
	//Set up aspect and field of view
	glPushMatrix();
	//glMatrixMode (GL_PROJECTION);
   	//glLoadIdentity ();
   	//glFrustum (leftCorner,rightCorner,bottom,top,win.z_near,win.z_far);
	//glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	//Draw Coordinate System
	if(1){
		//Draw enpoints and origin
		glBegin(GL_POINTS); // render with points
			glColor3f(1,0,1);
			glVertex3i(0,0,0); //display a point
			glColor3f(1,0,0);
			glVertex3f(0,3,0);
			glColor3f(0,1,0);
			glVertex3f(3,0,0);
			glColor3f(1,1,0);
			glVertex3f(0,0,3);
		glEnd();
	
		//Draw coordinate lines
		glBegin(GL_LINES);
			//y coordinate
			glColor3f(1,0,0);
			glVertex3f(0,0,0);
			glVertex3f(0,3,0);
			//x coordinate
			glColor3f(0,1,0);
			glVertex3f(0,0,0);
			glVertex3f(3,0,0);
			//z coordinate
			glColor3f(1,1,0);
			glVertex3f(0,0,0);
			glVertex3f(0,0,3);
		glEnd();
	}
	
	glLoadIdentity();
	
	//Save current matrix state
	glPushMatrix();	
		//Set cube color to RED							
		glColor3f(1,0,0);
		
		//Scale object by the size of the largest line segment. Calc in parseObj()
		glScalef(1/scale_factor,1/scale_factor,1/scale_factor);
		
		//Zoom in as needed
		glScalef(-g_fViewDistance,-g_fViewDistance,-g_fViewDistance);
		
		//TODO: Change it so that obj always rotates around center of obj
		//Rotate in the x direction
		glRotatef(hrotate,1,objCent_y,objCent_z);

		//Rotate in the y direction
		glRotatef(vrotate,objCent_x,1,objCent_z);

		//Translate the object to the origin
		glTranslatef(-((maxx+minx)/2),-((maxy+miny)/2),-((maxz+minz)/2));
		
		//move the object around
		glTranslatef(moveCords[0],moveCords[1],moveCords[2]);

		if(display_type.compare("q")==0){
			drawPoints();
		}
		else if(display_type.compare("w")==0){
			drawLines();
		}
		else if(display_type.compare("e")==0){
			drawPolygons();
		}
		else{
			drawPolygons();
		}

	glPopMatrix();
	
	glFlush();
	glutSwapBuffers();
}

void init(string filename) {
	//Select projection matrix
	glMatrixMode(GL_PROJECTION);
	//Set viewport: The final image projected down onto the display
	glViewport(0,0,win.width, win.height);
	
	//Reset projection matrix
	glLoadIdentity();
	//Calculate aspect ratio, ex 16:9 (ratio of width to height)
	aspect = (GLfloat) win.width/win.height;
	
	//Setup a perspective projection matrix
	gluPerspective(field_of_view_angle, aspect, win.z_near, win.z_far);
	
	//Look at origin (0,0,0) 
	//Put the camera 20 units down the x axis (20,0,0)//
	gluLookAt(1,0,-10, 0,0,0, 0,0,1);

	//Specify with matrix is the current matrix
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
   	glDepthFunc( GL_LEQUAL );
 	
   	//Lighting stuff
   	glEnable(GL_LIGHTING);
   	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHT0); 
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);

	glEnable(GL_LIGHT1); 
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos); 
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1); 
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular1);


	GLfloat shine = 100.0;

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);

	//Open file and fill data structures
	FILE* objectFile = fopen(filename.c_str(),"r");
	parseObjFile(objectFile);	
}
bool lightsOn = true;

void processKeys(unsigned char key, int x, int y) {
	//TODO: Turn this if-else nightmare into case-switch
	//Display mode select
	//Points
	if (key == 'q'){ display_type = "q"; }
	//Wireframe
	else 
		if(key == 'w'){ display_type = "w";	}
		//Polygons
		else 
			if(key == 'e'){ display_type = "e";	}
				else

	/*End display mode select*/
	//Move object
	if(key == 'o'){ //moveCords obj away
		moveCords[2] += 0.2f; //Down -z axis
		objCent_z += 0.2f;
	}else
	if(key == 'k'){ //Move obj <-
		moveCords[1] -= 0.2f;
		objCent_y -= 0.2f;
	}else
	if(key == 'l'){ // Move obj towards viewer
		moveCords[2] -= 0.2f;
		objCent_z -= 2.0f;
	}else 
	if(key == ';'){ //Move obj ->
		moveCords[1] += 0.2f;
		objCent_y += 0.2f;
	}else
	if(key == 'i'){ //Move up z axis
		moveCords[0] += 0.2f;
		objCent_x += 0.2f;
	}else
	if(key == 'p'){
		moveCords[0] -= 0.2f;
		objCent_x -= 0.2f;
	}
	else //Change aspect ration
	if(key == '.'){
		//aspect += 5f;
		//field_of_view_angle += 1;
		//cout << "field_of_view_angle ="<<field_of_view_angle<<endl;
	}
	else
	if(key == 's'){
		//aspect -= 5f;
		field_of_view_angle -= 1;
		
	}else
	if(key == ' '){
		if(lightsOn){
			glDisable(GL_LIGHTING);
			glFlush();
			lightsOn = false;
		}else{
			glEnable(GL_LIGHTING);
			lightsOn = true;
		}
	}else
	if(key == '-'){
		leftCorner -= 0.2f;
		bottom -= 0.2f;

		//rightCorner += 0.2;
	}else//Change RGB
	if(key == 'r'){ //Red levels
		ambient0[0]  += 0.1f;
		diffuse0[0]  += 0.15f;
		specular0[0] += 0.30f;
		diffuse1[0]  += 0.2f;
		ambient1[0]  += 0.1f;
		specular1[0] += 0.2f;
		ambient[0]   += 0.15f;
		diffuse[0]	 += 0.1f;
		specular[0]  += 0.3f;

	}else
	if(key == 'g'){ //Green levels
		ambient0[1]  += 0.1f;
		diffuse0[1]  += 0.15f;
		specular0[1] += 0.30f;
		diffuse1[1]  += 0.2f;
		ambient1[1]  += 0.1f;
		specular1[1] += 0.2f;
		ambient[1]   += 0.15f;
		diffuse[1]	 += 0.1f;
		specular[1]  += 0.3f;
	}else
	if(key == 'b'){ //Blue levels
		ambient0[2]  += 0.1f;
		diffuse0[2]  += 0.15f;
		specular0[2] += 0.30f;
		diffuse1[2]  += 0.2f;
		ambient1[2]  += 0.1f;
		specular1[2] += 0.2f;
		ambient[2]   += 0.15f;
		diffuse[2]	 += 0.1f;
		specular[2]  += 0.3f;
	}else
	if(key == 'a'){ //Ambient levels
		ambient0[3]  += 0.1f;
		diffuse0[3]  += 0.15f;
		specular0[3] += 0.30f;
		diffuse1[3]  += 0.2f;
		ambient1[3]  += 0.1f;
		specular1[3] += 0.2f;
		ambient[3]   += 0.15f;
		diffuse[3]	 += 0.1f;
		specular[3]  += 0.3f;
	}
	/*End of obj moving*/
    //Aspect ration, 
	//Calls the display function again
	glutPostRedisplay();
}
//TODO: Make the object rotation more fine tuned???
//int mouseMax_x = INT_MIN;
//int mouseMin_x = INT_MAX;
//int mouseMax_y = INT_MIN;
//int mouseMin_y = INT_MAX;

void drag(int x, int y){
 	//Check if user is trying to rotate object
	if(LeftIsPressed){
		
	  	//Calc v/h dis
	  	hdist = (x - oldX)/2;
	  	vdist = (y - oldY)/2;
	  	hrotate += hdist*0.05; //rotate by 10% of horizontal movement 
	  	vrotate += vdist*0.05; //rotate by 10% of vert movement

	    glutPostRedisplay();
    }else 
    	if(RightIsPressed){
    		//Zooming
	    	g_fViewDistance = (y - oldY) / 3.0;
	    	//if (g_fViewDistance < VIEWING_DISTANCE_MIN)
	    	//	g_fViewDistance = VIEWING_DISTANCE_MIN;

	    	glutPostRedisplay();
    	}
}

//For keeping track of previous mouse state
void processMouseClick(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        LeftIsPressed = (state == GLUT_DOWN) ? true : false;
		RightIsPressed = false;
	} 
	
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		RightIsPressed = (state == GLUT_DOWN) ? true : false;
		LeftIsPressed = false;
	}
    
    if(button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN){
 	  //TEST
 	  //cout << "processMouse:Middle button pressed" << endl;
    }
	oldX = x;
    oldY = y;
}

int main(int argc, char** argv){
	win.width = 700;
	win.height = 700;
	//win.field_of_view_angle = 90;
	win.z_near = 0.5f;
	win.z_far = 500.0f;
	
	//Set mouse state
	//oldX = 0;
	//oldY = 0;

	if(argc > 2){
		display_type = argv[1];
		
	}else if(argc == 2){
		filename = argv[1];
	}
	else{
		cout << "Error. Invalid input"<<endl;
	}

	glutInit(&argc, argv);
  	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  	glutInitWindowSize(win.width, win.width);
  	glutInitWindowPosition(200,1500);
  	glutCreateWindow("HW2");
  	glutKeyboardFunc(processKeys);
  	glutMouseFunc(processMouseClick);
  	glutMotionFunc(drag);
  	glutDisplayFunc(display);
  	//glutReshapeFunc(reshape);
  	init(filename);

  	//glutReshapeFunc(reshape);
  	glutMainLoop();
  return 0;
}
