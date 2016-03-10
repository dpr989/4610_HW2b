//#define WIN32

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <GL/glut.h>

using namespace std;

void parseObjFile(FILE* input);

//Vectors for stroing vertices
std::vector<GLfloat*> vertices;
std::vector<GLint*> faces;

int count = 0;
float g_rotation = 0;
float g_rotation_speed = 0.2f;
string display_type = "l";

//Vars for tracking the xyz min/max
int maxx = 0;
int maxy = 0;
int maxz = 0;
int minx = 0;
int miny = 0;
int minz = 0;

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
	size_t min = SIZE_MAX;
	
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
	cout << "\nThe max x:" << maxx << endl <<"The max y:"<< maxy << endl << "The max z:" << maxz << endl;
	cout << "\nThe min x:" << minx << endl <<"The min y:"<< miny << endl << "The min z:" << minz << endl;
	fclose(input);
}

void display(void){
	glClearColor(0.0,0.0,1.0,0.0); //Set clear color to blue
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
	gluLookAt(45,0,0, 0,0,0, 0,0,1);

	//glMatrixMode(GL_MODELVIEW);
	cout << "Center of object:"<<(maxx+minx)/2<<", "<<(maxy+miny)/2<<", "<<(maxz+minz)/2<<endl;
	glBegin(GL_POINTS);
		glVertex3f((maxx+minx)/2,(maxy+miny)/2,(maxz+minz)/2);
	glEnd();

	glPushMatrix();										
		glColor3f(1,0,0);
		
		//glScalef(0.5,0.5,0.5);
		glTranslatef(-((maxx+minx)/2),-((maxy+miny)/2),-((maxz+minz)/2));
		//cout <<endl<<"Translated x:"<<-((maxx+minx)/2)<<endl<<"Translated y:"<<-(maxy-miny)/2<<endl<<"Translated z:"<<-(maxz-minz)/2<<endl<<count;
		
		if(display_type.compare("p")==0){
			drawPoints();
		}
		else if(display_type.compare("l")==0){
			drawLines();
		}
		else if(display_type.compare("t")==0){
			drawPolygons();
		}
		else{
			drawPolygons();
		}
		
	glPopMatrix();
	g_rotation += g_rotation_speed;
	glutSwapBuffers();
}


void init(string filename) {
	//Select projection matrix
	glMatrixMode(GL_PROJECTION);
	//Set viewport
	glViewport(0,0,win.width, win.height);
	
	//Reset projection matrix
	glLoadIdentity();
	GLfloat aspect = (GLfloat) win.width/win.height;
	
	//Setup a perspective projection matrix
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
	
	//Specify with matrix is the current matrix
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
   	glDepthFunc( GL_LEQUAL );
 	glPointSize(10);

	//Open file and fill data structures
	FILE* objectFile = fopen(filename.c_str(),"r");
	parseObjFile(objectFile);	
}

void processKeys(unsigned char key, int x, int y) {
	if (key == 'a')
	// your code here
	glutPostRedisplay();
}

int main(int argc, char** argv){
	win.width = 700;
	win.height = 700;
	win.field_of_view_angle = 10;
	win.z_near = 1.0f;
	win.z_far = 500.0f;
	string filename;
	
	if(argc > 2){
		display_type = argv[1];
		filename = argv[2];
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
  	glutDisplayFunc(display);
  	init(filename);
  	//glutReshapeFunc(reshape);
  	glutMainLoop();
  return 0;
}
