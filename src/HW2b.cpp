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

float g_rotation = 0;
float g_rotation_speed = 0.2f;
string display_type = "t";
size_t maxx = 0;
size_t maxy = 0;
size_t maxz = 0;
size_t minx = 0;
size_t miny = 0;
size_t minz = 0;

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

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt( 60,0,0, 0,0,0, 0,0,1);

	glPushMatrix();										
		glColor3f(1,0,0);
		glTranslatef(2,0,0);							
		glRotatef(g_rotation,0,1,0);
		glRotatef(90,0,1,0);
		
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

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
   glMatrixMode (GL_MODELVIEW);
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
	// specify implementation-specific hints
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );						
 
	GLfloat amb_light[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
    GLfloat specular[] = { 0.7, 0.7, 0.3, 1 };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    glEnable( GL_LIGHT0 );
    glEnable( GL_COLOR_MATERIAL );
    glShadeModel( GL_SMOOTH );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); 
	glClearColor(0.0, 0.0, 0.0, 1.0);
   	//glShadeModel (GL_FLAT);

	//Open each file
	FILE* cube = fopen(filename.c_str(),"r");
	parseObjFile(cube);	
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
	//cout << endl <<argv[1]<<endl;
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
  	glutCreateWindow("HW2");
  	glutDisplayFunc(display);
  	init(filename);
  	//glutReshapeFunc(reshape);
  	glutMainLoop();
  return 0;
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
			if(f1>minx){
				minx = f1;
			}
			if(f2>miny){
				miny=f2;
			}
			if(f3>minz){
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
	fclose(input);
}