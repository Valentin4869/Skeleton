// Skeleton_002.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>
#include <vector>
#include <iterator>
#include <sstream>
#include "Glut\glut.h"
#include <strsafe.h>
#include <algorithm>
#include <time.h>
//#define pair(i) i + (int)(ceil((files_N - 1))) / 2 + 1
#define pair(i) i + files_N / 2 

char dirpath_c[] = "E:\\csv\\list8\\";
#pragma comment(lib,"glut32.lib")

//show 17, 23, 21 from list8

bool leftButton = false, middleButton = false, rightButton = false;
float sphi = 30.0, stheta = -30.0, sheight = 0, shor = 0;
int downX, downY;
float sdepth = 20;
bool pause = false;;
bool keypress = false;
bool keypress2 = false;
bool print = false;
float lightx = 1, lighty = 1, lightz = 1;
double t_scale = 20;
void Render();
void Idle();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void SpecialKey(int k, int x, int y);
void key_categorize(int k, int x, int y);
void keyboard(unsigned char key, int x, int y);

using namespace std;

struct vector3d {
	double x;
	double y;
	double z;
	vector3d() { x = 0; y = 0; z = 0; }
	vector3d(double ix, double iy, double iz) { x = ix; y = iy; z = iz; }
};

struct skeleton {

	short n_joints;
	vector3d *dxyz; //joint displacement

	skeleton(int n) { n_joints = n; dxyz = new vector3d[n_joints]; }
	~skeleton() { delete[] dxyz; }

};


void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, 0, 0);
	gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 100000000000.0);
	glMatrixMode(GL_MODELVIEW);

}
void InitGL(int* argc, char** argv)
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInit(argc, argv);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(420, 60);
	glutCreateWindow("Skeleton");


	glShadeModel(GL_SMOOTH);
	GLfloat light_position[] = { lightx, lighty, lightz, 0.0 };
	glClearColor(0.0f, 0.1f, 0.2f, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	GLfloat ambientColor[] = { 0.0f, 0.1f, 0.2f, 0.0f };
	GLfloat diffuseColor[] = { 0.15f, 0.15f, 0.2f, 0.0f };
	GLfloat specularColor[] = { 0.0f, 0.0f, 0.2f, 0.0f };
	GLfloat position[] = { 100.0f, 100.0f, 400.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);



	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutDisplayFunc(Render);
	glutReshapeFunc(reshape);
	glutSpecialFunc(key_categorize);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(Idle);



}

vector<vector<double>> readCSV(const char* path, bool print) {


	ifstream in;
	string line;
	string cline;
	vector< vector<double> > X;

	
	in.open(path);
	assert(in.is_open());

	while (getline(in, line)) {

		istringstream ss(line);
		istream_iterator<string> begin(ss), end;
		vector<string> arrayTokens(begin, end);
		auto it = arrayTokens.begin();


		cline = *it;
		if (print)
			cout << "Processing line:\n" << cline << endl;

		bool foundcomma = true;
		int lastoffset = 0;
		int lastcomma = 0;
		vector<double> row;
		while (foundcomma) {

			lastcomma = cline.find(',', lastoffset);


			if (lastcomma >= 0) {


				row.push_back(stod(cline.substr(lastoffset, lastcomma - lastoffset)));
				lastoffset = lastcomma + 1;

			}
			else {

				foundcomma = false;
				row.push_back(stod(cline.substr(lastoffset, cline.length() - lastoffset)));

			}

		}
		X.push_back(row);
	}

	in.close();


	//print
	if (print) {
		cout << "\n\nelements:" << X.size() << "x" << X.front().size() << endl;
		for (int i = 0; i < X.size(); i++)
			for (int j = 0; j < X.front().size(); j++)
				printf("%i,%i: %f\n", i, j, X[i][j]);
	}


	return X;
}


vector< vector<double> > dxyz;
vector< vector<double> > pred_dxyz;
vector< vector<double> > seed_dxyz;
vector< vector<double> > parents;
vector< vector<double> > nestdepth;
skeleton s(64);
vector<string> fname_list;
vector<WIN32_FIND_DATAA> ffd_list;
bool pred = 1;
bool seed = 0;
int frame_id = 0;
bool loadnewfile = true;
int file_id = 0;
int frames_N = 60;
int frames_seed_N = 60;
int frames_pred_N = 60;
int fps = 30;
int files_N = 0;
char *categ_line = nullptr;
ofstream categ_file;
clock_t clock_start;

int main(int argc, char* argv[])
{

	WIN32_FIND_DATAA ffd;
	char szDir[MAX_PATH];

	HANDLE hFind = INVALID_HANDLE_VALUE;
	LPCWSTR dirpath = L"K:\\MoCap\\Data\\csv\\";
//	char dirpath_c[] = "K:\\MoCap\\Data\\csv\\";
	



	StringCchCopyA(szDir, MAX_PATH, dirpath_c);
	StringCchCatA(szDir, MAX_PATH, "*");

	hFind = FindFirstFileA(szDir, &ffd);


	while (FindNextFileA(hFind, &ffd)) {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}
		else
		{
			files_N++;
			ffd_list.push_back(ffd);

			char fullpath[MAX_PATH];
			StringCchCopyA(fullpath, MAX_PATH, dirpath_c);
			StringCchCatA(fullpath, MAX_PATH, ffd.cFileName);
		
			fname_list.push_back(fullpath);
			fname_list[files_N - 1] = fullpath;
			

		}
	}



	//StringCchCopy(fullpath, MAX_PATH, dirpath);
	//StringCchCat(fullpath, MAX_PATH, ffd_list[4].cFileName);
	cout << "Found " << files_N << ", size:" << fname_list.size()<<endl;
	
	//for (int i = 0; i < (files_N - 1) / 2; i++)
	//{
	//	cout<< "-------\n";
	//	cout << i << ": ";
	//	cout<<fname_list[i] << endl;
	//	cout << i + pair(i);
	//	cout<< ": " << fname_list[pair(i)] << endl;
	//	//system("pause");

	//}
	//wcout << "Full path: " << fullpath << endl;



	parents = readCSV("parents2.csv", 0);
	
	nestdepth = readCSV("nestdepth.csv", 0);
	
	parents[0][0] = 1;
	





	InitGL(&argc, argv);
	glutMainLoop();

	
	return 0;
}




void Render()
{

	clock_start = clock();

	vector3d v1(1.0, 2.0, 1.5);
	vector3d v2(3.0, 3.8, 3.0);
	vector3d v3(2.0, 1.8, 1.5);
	vector3d Yn(0, 1, 0);
	vector3d Xn(1, 0, 0);
	vector3d Zn(0, 0, 1);





	glPushMatrix();
	glTranslatef(-shor, sheight, -sdepth);
	glRotatef(-stheta, 1, 0, 0);
	glRotatef(sphi, 0, 1, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	if (loadnewfile)
	{

		cout << "Loading " << fname_list[file_id] << "...\n";
		pred_dxyz = readCSV(fname_list[file_id].c_str(), 0);

		cout << "Loading " << fname_list[pair(file_id)] << "...\n";
		seed_dxyz = readCSV(fname_list[pair(file_id)].c_str(), 0);

		loadnewfile = false;
		frame_id = 0;
		frames_N = seed_dxyz.front().size();
		frames_seed_N = seed_dxyz.front().size();
		frames_pred_N = pred_dxyz.front().size();
		dxyz = seed_dxyz;
		seed = true;
		pred = false;

	}

	//When first sequence ends
	if (frame_id < frames_N - 1)
	{
		if (!pause)
			frame_id++;
	}
	else
	{

		if (seed)
		{
			seed = false;
			pred = true;
			frames_N = frames_pred_N;
			dxyz = pred_dxyz;
			
		}
		else
		{
			seed = true;
			pred = false;
			frames_N = frames_seed_N;
			dxyz = seed_dxyz;
		}

		frame_id = 0;
	}





	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);



	for (int i = 0; i < s.n_joints; i++) {

		int parent_id = (int)parents[i][0] - 1;
		s.dxyz[i].x = t_scale*dxyz[i * 3 + 0][frame_id];
		s.dxyz[i].y = t_scale*dxyz[i * 3 + 1][frame_id];
		s.dxyz[i].z = t_scale*dxyz[i * 3 + 2][frame_id];





		glPushMatrix();


		//direction vector to child/parent
		vector3d dvec(s.dxyz[i].x - s.dxyz[parent_id].x, s.dxyz[i].y - s.dxyz[parent_id].y, s.dxyz[i].z - s.dxyz[parent_id].z);
		double magdvec = sqrt(dvec.x*dvec.x + dvec.y*dvec.y + dvec.z*dvec.z);

		
		double az = atan2(dvec.y, dvec.x)*57.29578;
		double el = acos(dvec.z / magdvec)*57.29578;




		glTranslatef(s.dxyz[parent_id].x + dvec.x / 2, s.dxyz[parent_id].y + dvec.y / 2, s.dxyz[parent_id].z + dvec.z / 2);
		
		double sc = 0.2*(log2(nestdepth[i][0]) / log2(4));
		glRotatef(az, 0, 0, 1);
		glRotatef(el, 0, 1, 0);
		glScalef(0.5 - sc, 0.5 - sc, magdvec);


		if(seed)
		glColor3f(1, 1, 1);
		else
		glColor3f(0.05, 0.95, 0.35);

		glutSolidCube(1);
		glPopMatrix();










	}

	glLineWidth(2.5);
	glBegin(GL_QUADS);




	
	glColor3f(0.1, 0.40, 0.6);
	float ground = 80;
	float elev = -0.2;
	glVertex3d(-ground, elev, -ground);
	glColor3f(0.1, 0.1, 0.7);
	glVertex3d(-ground, elev, ground);
	glColor3f(0.1, 0.1, 0.4);
	glVertex3d(ground, elev, ground);
	glColor3f(0.1, 0.2, 0.2);
	glVertex3d(ground, elev, -ground);
	glEnd();
glEnd();
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);



	glColor3f(1.0f, 0, 0);


	glVertex3f(0, 0, 0);
	glVertex3f(10.f, 0.f, 0.f);


	glColor3f(0, 1.0f, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 10, 0);
	glColor3f(0, 0.0, 1.0f);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 10);

	glEnd();

	
	glPopMatrix();




	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (double)800, 0.0, (double)600, -1.0, 1.0);

	// Move to modelview mode.
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	stringstream ss;
	stringstream text_cats;
	ss << frame_id << "/" << frames_N << "\t\t\t\t\t\t\t\tFPS: "<<fps<<"\t\t\t\t\t\t file_id: " << file_id;

	char text[80];
	

	ss.getline(text, 80);
	//text_cats.getline(cats_list, 200);


	int length = strlen(text);
	//int length2 = strlen(cats_list);
	float xAlignment = 0, yAlignment = 0;
	

	glColor3f(1, 1, 1);
	for (int i = 0; i<length; i++)
	{

		if (text[i] == '\n')
		{
			xAlignment = 0; yAlignment += 13; continue;
		}
		if (text[i] == '\t')
		{
			xAlignment += 40; continue;
		}

		glRasterPos2f(10 + xAlignment, 10 - yAlignment);
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text[i]);
		xAlignment += 8;
	}

	xAlignment = 0;
	yAlignment = 0;

	glColor3f(1, 1, 1);
	/*for (int i = 0; i<length2; i++)
	{

	if (cats_list[i] == '\b')
	{
	xAlignment = 0; yAlignment += 13; continue;
	}
	if (cats_list[i] == '\t')
	{
	xAlignment += 35; continue;
	}

	glRasterPos2f(160+ xAlignment, 10- yAlignment);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, cats_list[i]);
	xAlignment += 8;
	}*/



	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);


	while (((double(clock() - clock_start) / double(CLOCKS_PER_SEC)) < 1.0 / double(fps)) ){}
	glFlush();
	glutSwapBuffers();
}

void Idle()
{

	glutPostRedisplay();
}


void motion(int x, int y)
{
	static int temp_x = x, temp_y = y;
	static float margin = 0.05;



	if (leftButton)
	{
		sphi += (float)(x - downX) / 4.0;
		stheta += (float)(downY - y) / 4.0;
	}
	if (rightButton) {
		if (sdepth <= 2 && sdepth >= -2)
			sdepth += (float)(downY - y);

		else sdepth += (float)(downY - y)*(abs(sdepth)) / 50.0;
		

	} // scale


	if (middleButton)
	{
		sheight += (float)(downY - y)*(abs(sdepth)) / 120.0;
		shor += (float)(downX - x)*(abs(sdepth)) / 120.0;
	}

	downX = x;   downY = y;


}

void mouse(int button, int state, int x, int y)
{
	downX = x; downY = y;
	leftButton = ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN));

	middleButton = ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN));

	rightButton = ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN));

}

void SpecialKey(int k, int x, int y)
{

	switch (k)
	{
	case GLUT_KEY_UP:



	case GLUT_KEY_DOWN:
		lighty -= 0.1;

		break;

	case GLUT_KEY_RIGHT:
		keypress2 = !keypress2;

		break;

	case GLUT_KEY_LEFT:

		keypress = !keypress;
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case '+':
		fps += 10;
		break;

	case '-':
		fps = max(1, fps- file_id - 10);
		break;

	case '.':
		frame_id++;
		break;

	case ',':
		frame_id--;
		break;

	case ']':
		file_id = min(pair(files_N - 1), file_id + 10);
		loadnewfile = true;

		break;
	case 'p':
		pause = !pause;
		break;
		
	case '[':
		file_id = max(-1, file_id - 10);
		loadnewfile = true;


		break;

		//backspace

	case 8:
	

		break;

		// enter
	case 13:
	
		break;
	default:
		break;
	}

}

void key_categorize(int k, int x, int y)
{

	switch (k)
	{
	case GLUT_KEY_UP:
		frame_id = min(frames_N - 2, frame_id + 20);
		break;

	case GLUT_KEY_DOWN:
		frame_id = max(0, frame_id - 20);

		break;

	case GLUT_KEY_RIGHT:
		file_id = min(pair(files_N - 1), file_id + 1);
		loadnewfile = true;


		break;

	case GLUT_KEY_LEFT:
		file_id = max(-1, file_id - 1);
		loadnewfile = true;
		break;
	}
}
