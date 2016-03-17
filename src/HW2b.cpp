//#define WIN32

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <climits>
#include <GL/glut.h>
#include <sys/stat.h>

#define VIEWING_DISTANCE_MIN  3.0
using namespace std;

void parseObjFile(FILE* input);

//Vectors for stroing vertices
std::vector<GLfloat*> vertices;
std::vector<GLint*> faces;

int count = 0;
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
int vdist;
int hdist;

//State of mouse click 
bool LeftIsPressed = false;
bool RightIsPressed = false;

//Vertical rotation
int hrotate = 0;
int vrotate = 0;

GLfloat zoom_factor = 1;
static GLfloat moveCords[] = {0.0,0.0,0.0};

struct glutWindow{
	int width;
	int height;
	string title;
	float field_of_view_angle;
	size_t z_near;
	size_t z_far;
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
			glVertex3fv(vertices[faces[i][0] - 1]);
			glVertex3fv(vertices[faces[i][1] - 1]);
			glVertex3fv(vertices[faces[i][2] - 1]);
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

void display(void){
	glClearColor(1.0,1.0,1.0,0.0); //Set clear color to blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear color buffer and depth buffer
	glMatrixMode(GL_MODELVIEW);
	
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
		glScalef(g_fViewDistance,g_fViewDistance,g_fViewDistance);
		
		//Rotate in the x direction
		glRotatef(hrotate,1,0,0);

		//Rotate in the y direction
		glRotatef(vrotate,0,1,0);

		//Translate the object to the origin
		glTranslatef(-((maxx+minx)/2),-((maxy+miny)/2),-((maxz+minz)/2));
		
		//
		//glTranslatef(moveCords[0],moveCords[1],moveCords[2]);

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
	GLfloat aspect = (GLfloat) win.width/win.height;
	
	//Setup a perspective projection matrix
	gluPerspective(80/*win.field_of_view_angle*/, aspect, win.z_near, win.z_far);
	
	//Look at origin (0,0,0) 
	//Put the camera 20 units down the x axis (20,0,0)//
	gluLookAt(1,0,-20, 0,0,0, 0,0,1);

	//Specify with matrix is the current matrix
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
   	glDepthFunc( GL_LEQUAL );
 	

	//Open file and fill data structures
	FILE* objectFile = fopen(filename.c_str(),"r");
	parseObjFile(objectFile);	
}

void processKeys(unsigned char key, int x, int y) {
	//Points
	//cout << "Processkeys: We have arrived! " << endl;
	if (key == 'q'){
		display_type = "q";
	}
	//Wireframe
	else 
		if(key == 'w'){
			display_type = "w";
		}
		//Polygons
		else 
			if(key == 'e'){
				display_type = "e";
			}
			else
				if(key == 'o'){ //moveCords obj away
					moveCords[2] -= 0.2f; //Down -z axis
				}else
					 if(key == 'k'){ //Move obj <-
					 	moveCords[1] -= 0.2f;
					 }else
					 	  if(key == 'l'){ // Move obj towards viewer
					 	  	moveCords[2] += 0.2f;
					 	  }else 
					 	  	   if(key == ';'){ //Move obj ->
					 	  	   		moveCords[1] += 0.2f;
					 	  	   }
	//Calls the display function again
	glutPostRedisplay();
}

int mouseMax_x = INT_MIN;
int mouseMin_x = INT_MAX;
int mouseMax_y = INT_MIN;
int mouseMin_y = INT_MAX;

void drag(int x, int y){
 
	if(LeftIsPressed){
		
	  	//Calc v/h dis
	  	hdist = (x - oldX);
	  	vdist = (y - oldY);
	  	hrotate += hdist*0.05; //rotate by 10% of horizontal movement 
	  	vrotate += vdist*0.05; //rotate by 10% of vert movement

	  	//TEST
	  	cout << "Left is pressed" << endl;
	    glutPostRedisplay();
    }else 
    	if(RightIsPressed){
    		//Zooming
	    	g_fViewDistance = (y - oldY) / 3.0;
	    	if (g_fViewDistance < VIEWING_DISTANCE_MIN)
	    		g_fViewDistance = VIEWING_DISTANCE_MIN;

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
	win.field_of_view_angle = 90;
	win.z_near = 0.5f;
	win.z_far = 500.0f;
	
	//Set mouse state
	oldX = 0;
	oldY = 0;

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
  	init(filename);

  	//glutReshapeFunc(reshape);
  	glutMainLoop();
  return 0;
}
