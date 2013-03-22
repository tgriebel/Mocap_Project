#include <iostream>
#include <time.h>
#include <sstream>
#include <cstring>
#include <cctype>

using namespace std;

#include <gl\glut.h>

#include <iostream>
using namespace std;

#include <gl\glut.h>
#include <vector>

#define ESC 0x1B

#include "texture.h"
//#include "animation.h"
#include "UI.h"
#include "global.h"
#include "FreeCamera.h"
#include "BVHParser.h"
#include "Node.h"
#include "Quaternion.h"

bool quit = false;

void myInit();
#include "menu.h"

unsigned gW, gH;
unsigned TIMER_MS = 25;// 25 milliseconds makes the display update every 24 times/second
unsigned passed_time = 0;
const unsigned draw_dist = 40;
unsigned selected_stream = 0;

struct Pose;

BVHParser parser;
Animation* figure1, *figure2;
Pose* pose;

vector<vec2d> splices;

unsigned frame = 0;
unsigned fps = 0;
int motionIndex = 0;

vec3d lastKnownRootPos;
//vector<double> motionData = parser.getMotions("Example1.bvh");

UI *ui;

FreeCamera* camera;//use reference to change between free cam and target cam easily

bool cullback = true;
bool FREECAM_MODE = false;
bool LEFTMOUSE_DOWN = false;
bool ORBIT_MODE = false;
bool wireframe_flag = false;
bool play = false;
bool prevDown = false;
bool nextDown = false;
bool commandMode = false;

TextureID checkerTex;

void specialKeys(int key, int x, int y);
void myKeyboard(unsigned char key, int x, int y);
void update(int value);
void mouseAim(int x2, int y2);
void mouse(int btn, int state, int x, int y);
void computePos( node* n, double p[]);
void writeBVH( const string& fileName, Animation* ani);
void copyFrame( nodeTransform* nT, nodeTransform*& newParent, unsigned childNum = 0);
void interpFrame(  nodeTransform* frame1, nodeTransform* frame2, nodeTransform*& newParent, unsigned childNum, float weight);

void toggleFreeCamMode(){
	FREECAM_MODE ^= true;
	ORBIT_MODE = false;
}

void toggleOrbitCamMode(){
	ORBIT_MODE ^= true;
	FREECAM_MODE = false;
}

void togglePlay(){
	play ^= true;
}

void nextFrame(){
	nextDown = true;
	if(figure1->currentFrame < (figure1->numFrames) - 1){
		figure1->currentFrame++;
	}else{
		figure1->currentFrame = 0;
	}
}

void prevFrame(){
	prevDown = true;
	if(figure1->currentFrame > 0 ){
		figure1->currentFrame--;
	}
}


void display(){}
void clearWindow(){
	glClearColor(0.0, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PerspectiveSet(int w, int h, double fov){
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glViewport(0, 78, w, h);
	gluPerspective(fov,(w/(h*1.0)),.01,draw_dist);
    
    glMatrixMode(GL_MODELVIEW);
}
void OrthoSet(int w, int h){}
void myReshape(int w, int h)
{

	gW = w;
	gH = h;

	PerspectiveSet(w, h, camera->fov);

	ui->resize(gW, gH);

}

void drawCube(double x, double y, double z, double size){

	float color[4] = {1.0,0.0,0.0,0.5};
	glColor4fv(color);

	glNormal3f(1.0,0.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x+size, y+size, z);
		glVertex3f(x+size, y+size, z+size);
		glVertex3f(x+size, y, z+size);
		glVertex3f(x+size,y,z);	
	glEnd();

	glNormal3f(-1.0,0.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x, y, z+size);
		glVertex3f(x, y+size, z+size);
		glVertex3f(x, y+size, z);
		glVertex3f(x,y,z);
	glEnd();

	glNormal3f(0.0,0.0,-1.0);
	glBegin(GL_QUADS);
		glVertex3f(x+size, y, z+size);
		glVertex3f(x+size, y+size, z+size);
		glVertex3f(x, y+size, z+size);
		glVertex3f(x,y,z+size);	
	glEnd();
	
	glNormal3f(0.0,0.0,1.0);
	glBegin(GL_QUADS);
		glVertex3f(x, y+size, z);
		glVertex3f(x+size, y+size, z);
		glVertex3f(x+size, y, z);
		glVertex3f(x,y,z);
	glEnd();

	glNormal3f(0.0,-1.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x, y+size, z);
		glVertex3f(x+size, y+size, z);
		glVertex3f(x+size, y+size, z+size);
		glVertex3f(x,y+size,z+size);
	glEnd();

	glNormal3f(0.0,1.0,0.0);
	glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x+size, y, z);
		glVertex3f(x+size, y, z+size);
		glVertex3f(x,y,z+size);
	glEnd();

}

void loadAnimation( const string& fileName, Animation*& ani ){

	cout << "Loading Figure: " << fileName << "..." << endl;

	//if( ani != NULL )	delete ani;
	
	ani = parser.getData( fileName.c_str() );
}

void myInit()
{
	loadAnimation( "newIn.bvh", figure1 );
	loadAnimation( "03_02.bvh", figure2 );

	pose = new Pose;

	cullback = true;
	FREECAM_MODE = false;
	wireframe_flag = false;
	quit = false;

	glClearColor(1.0, .5, .5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutDisplayFunc(display);
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(specialKeys);
	glutReshapeFunc(myReshape);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouseAim);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);

	TextureID textID;
	loadTexture32("UI.bmp", textID);

	ui = new UI(gH, gW, textID);

	loadTexture32("cam_but.bmp", textID);
	ui->buttons[0].frames[0] = textID;
	loadTexture32("cam_but_down.bmp", textID);
	ui->buttons[0].frames[1] = textID;
	ui->buttons[0].x = 8;
	ui->buttons[0].y = 8;
	ui->buttons[0].h = 64;
	ui->buttons[0].w = 64;
	ui->buttons[0].frame = 0;
	ui->buttons[0].click = toggleFreeCamMode;
	ui->buttons[0].global_control = &FREECAM_MODE;
	ui->numButs++;
	
	loadTexture32("orbit_but.bmp", textID);
	ui->buttons[1].frames[0] = textID;
	loadTexture32("orbit_but_down.bmp", textID);
	ui->buttons[1].frames[1] = textID;
	ui->buttons[1].x = 76;
	ui->buttons[1].y = 8;
	ui->buttons[1].h = 64;
	ui->buttons[1].w = 64;
	ui->buttons[1].frame = 0;
	ui->buttons[1].click = toggleOrbitCamMode;
	ui->buttons[1].global_control = &ORBIT_MODE;
	ui->numButs++;
	
	loadTexture32("prev_but.bmp", textID);
	ui->buttons[2].frames[0] = textID;
	loadTexture32("prev_but_down.bmp", textID);
	ui->buttons[2].frames[1] = textID;
	ui->buttons[2].x = 424;
	ui->buttons[2].y = 8;
	ui->buttons[2].h = 64;
	ui->buttons[2].w = 64;
	ui->buttons[2].frame = 0;
	ui->buttons[2].click = prevFrame;
	ui->buttons[2].global_control = &prevDown;
	ui->numButs++;

	loadTexture32("next_but.bmp", textID);
	ui->buttons[3].frames[0] = textID;
	loadTexture32("next_but_down.bmp", textID);
	ui->buttons[3].frames[1] = textID;
	ui->buttons[3].x = 492;
	ui->buttons[3].y = 8;
	ui->buttons[3].h = 64;
	ui->buttons[3].w = 64;
	ui->buttons[3].frame = 0;
	ui->buttons[3].click = nextFrame;
	ui->buttons[3].global_control = &nextDown;
	ui->numButs++;

	loadTexture32("play_but.bmp", textID);
	ui->buttons[4].frames[0] = textID;
	loadTexture32("pause_but.bmp", textID);
	ui->buttons[4].frames[1] = textID;
	ui->buttons[4].x = 560;
	ui->buttons[4].y = 8;
	ui->buttons[4].h = 64;
	ui->buttons[4].w = 64;
	ui->buttons[4].frame = 0;
	ui->buttons[4].click = togglePlay;
	ui->buttons[4].global_control = &play;
	ui->numButs++;
	
	loadTexture32("checker.bmp", checkerTex);

	camera = new FreeCamera(0.5, 0.75, 0.5);
	
	glutTimerFunc(TIMER_MS, update, 0);
	glutPostRedisplay();

	ui->totalFrames = &(figure1->numFrames);
	ui->currentFrame = &(figure1->currentFrame);
	ui->fps = &fps;

	fps = 1.0f/figure1->frameRate;
	TIMER_MS = 1000 / fps;

	/*
	for( int i(100); i < 200; ++i){
		nodeTransform* nT(NULL);
		copyFrame( figure2->motionVector[ i ],  nT);
		figure1->motionVector[ i ] = nT;
	}*/

	//writeBVH( "TomTomTest.bvh", figure1);

	for( int i(0); i < 200; i += 2){
		nodeTransform* nT(NULL);
		interpFrame(  figure1->motionVector[ i ], figure2->motionVector[ i + 1], nT, 0, .01f);

		figure1->motionVector.insert( figure1->motionVector.begin() + (i + 1), nT);
		figure1->motionVector[i + 1]->interp = true;

		//figure1->figure->startingPos
	}

	figure1->numFrames = figure1->motionVector.size();
}

void mouse(int btn, int state, int x, int y){

	nextDown = false; prevDown = false;

	y = wh - y;
	if(btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN){

		LEFTMOUSE_DOWN = true;
		ui->mouseClick(x, y);
	}else{
		LEFTMOUSE_DOWN = false;
	}
}
void mouseAim(int x2, int y2){

	ui->mouseMove( x2, gH - y2);
	//LEFTMOUSE_DOWN &&
	if( (FREECAM_MODE || ORBIT_MODE)){

		glutSetCursor (GLUT_CURSOR_NONE);
			
		static int x1(0), y1(0);
		static int resetCnt(0);

		double dx = (double)x2 - (double)(gW/2.0);
		double dy = (double)y2 - (double)(gH/2.0);

		double theshold = 8;

		if(resetCnt > 1){//100 cycles

			if(dx > 0 && dx > theshold){

				if(ORBIT_MODE)
					camera->orbitX(-2);
				else
					camera->panX(-2);

			}else if (dx < -theshold){

				if(ORBIT_MODE)
					camera->orbitX(2);
				else
					camera->panX(2);
			}

			if(dy < 0  && dy < -theshold){

				if(ORBIT_MODE)
					camera->orbitY(2);
				else
					camera->panY(2);

			}else if (dy > theshold){

				if(ORBIT_MODE)
					camera->orbitY(-2);
				else
					camera->panY(-2);
			}

			glutWarpPointer  ( gW/2.0 , gH/2.0 );
			resetCnt = 0;
		
		}else{
			resetCnt++;
		}
	}else{
		glutSetCursor (GLUT_CURSOR_CROSSHAIR);
	}
}
void specialKeys(int key, int x, int y){

	switch(key){
		case GLUT_KEY_UP:
			camera->truckY(.1);
			break;
		case GLUT_KEY_DOWN:
			camera->truckY(-.1);
			break;
		case GLUT_KEY_LEFT:
			camera->truckX(-.1);
			break;
		case GLUT_KEY_RIGHT:
			camera->truckX(.1);
			break;
		case GLUT_KEY_F1:
			cullback ^= true;
			break;
		case GLUT_KEY_F2:
			wireframe_flag ^= true;
			break;
	}
}

void myKeyboard(unsigned char key, int x, int y){

	if(key == ESC) quit = true;

	tolower(key);
	switch(key){
	
		case 'i':
			toggleFreeCamMode();
			break;
		case 'o':
			toggleOrbitCamMode();
			break;
		case 'a':
			camera->panX(5.);
			break;
		case 'd':
			camera->panX(-5.);
			break;
		case 'r':
			camera->panY(5.);
			break;
		case 'f':
			camera->panY(-5.);
			break;
		case 's':
			camera->dolly(-.1);
			break;
		case 'w':
			camera->dolly(.1);
			break;
		case 'q':
			camera->roll(2.);
			break;
		case 'e':
			camera->roll(-2.0);
			break;
		case 't':
			camera->dollyTarget(.1);
			break;
		case 'g':
			camera->dollyTarget(-.1);
			break;
		case 'y':
			camera->dollyCamera(.1);
			break;
		case 'h':
			camera->dollyCamera(-.1);
			break;
		case '[':
			camera->orbitX(-2.);
			break;
		case ']':
			camera->orbitX(2.);
			break;
		case 'p':
			camera->orbitY(2.);
			break;
		case ';':
			camera->orbitY(-2.);
			break;
		case 'v':
			camera->fov++;
			break;
		case 'b':
			camera->fov--;
			break;
		case 'm':

			commandMode = true;
			break;
		case 'n':

			if( selected_stream >= (figure1->numJoints() -1 )){
	
				selected_stream = 0;
			}else{
		
				selected_stream++;
			}
			break;
		default:
			break;
	}
}

void drawBone(vec3d& p, vec3d& axis, double degree, double length){

	glPushMatrix();

	double minY = 0.0f;
	double maxY = 1.0f;
	double minX = -0.5f;
	double maxX = 0.5f;
	double minZ = -0.5f;
	double maxZ = 0.5f;
	double plane = fabs(maxY - minY) / 4.0f;
	double cX = (fabs(maxX - minX) + minX) / 2.0f;
	double cY = (fabs(maxY - minY) + minY) / 2.0f;
	double cZ = (fabs(maxZ - minZ) + minZ) / 2.0f;

	glTranslatef( p[0], p[1], p[2]);
	glRotatef( degree, axis[0], axis[1], axis[2]);
	glScalef( length/10.0f, length, length/10.0f);

	glBegin(GL_POLYGON);
		glColor4f(1.f, .5f, 0.5f, .5f);
		
		glVertex3f( minX, plane, minZ);
		glVertex3f( minX, plane, maxZ);

		glVertex3f( 0, 1.0, 0);
	glEnd();
	glBegin(GL_POLYGON);
		glColor4f(.5f, .5f, 1.f, .5f);
		
		glVertex3f( minX, plane, maxZ);
		glVertex3f( maxX, plane, maxZ);

		glVertex3f( 0.0, 1.0, 0.0);
	glEnd();
	glBegin(GL_POLYGON);
		glColor4f(.5f, 1.f, 1.f, .5f);
		
		glVertex3f( maxX, plane, maxZ);
		glVertex3f( maxX, plane, minZ);

		glVertex3f( 0.0, 1.0, 0.0);
	glEnd();
	glBegin(GL_POLYGON);
		glColor4f(.5f, 1.f, 0.5f, .5f);
		
		glVertex3f( maxX, plane, minZ);
		glVertex3f( minX, plane, minZ);

		glVertex3f( 0.0, 1.0, 0.0);
	glEnd();
	/////////////
	glBegin(GL_POLYGON);
		glColor4f(.5f, .5f, .5f, 1.f);
		glVertex3f( minX, plane, minZ);
		glVertex3f( maxX, plane, minZ);
		glVertex3f( maxX, plane, maxZ);
		glVertex3f( minX, plane, maxZ);
	glEnd();

	/////
	glBegin(GL_POLYGON);
		glColor4f(1.f, .5f, 0.5f, .5f);
		
		glVertex3f( minX, plane, minZ);
		glVertex3f( maxX, plane, minZ);
		glVertex3f( 0.0, 0.0, 0.0);

	glEnd();
	glBegin(GL_POLYGON);
		glColor4f(.5f, 1.f, 0.5f, .5f);

		glVertex3f( maxX, plane, minZ);
		glVertex3f( maxX, plane, maxZ);
		glVertex3f( 0.0, 0.0, 0.0);
	glEnd();
	glBegin(GL_POLYGON);
		glColor4f(.5f, 1.f, 1.f, .5f);
		
		glVertex3f( maxX, plane, maxZ);
		glVertex3f( minX, plane, maxZ);
		glVertex3f( 0.0, 0.0, 0.0);
	glEnd();
	glBegin(GL_POLYGON);
		glColor4f(.5f, .5f, 1.f, .5f);
		
		glVertex3f( minX, plane, maxZ);
		glVertex3f( minX, plane, minZ);
		glVertex3f( 0.0, 0.0, 0.0);
	glEnd();
	
	glPopMatrix();
	
}

void updateRot( node& n, nodeTransform& nT, float theta, const vec3d& ori, vec3d& axis){

	vec3f p( n.pos );

	p -= ori;

	rotate( theta, axis, nT.Z);
	rotate( theta, axis, nT.X);
	rotate( theta, axis, nT.Y);

	rotate( theta, axis, p);

	for( unsigned k = 0; k < n.numChildren; ++k){

		updateRot( *n.children[k], *nT.children[k], theta, ori, axis);
	}

	p += ori;
	n.pos = p;
}

void drawPose( const Pose& pose){

	for( unsigned i(0); i < pose.lines.size(); ++i){
	
		glBegin(GL_LINES);
			glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
			
			const vec3f& pt1 = pose.vlines[i].first;
			const vec3f& pt2 = pose.vlines[i].second;

			//const vec3f& pt1 = pose.pts[ pose.lines[i].first ];
			//const vec3f& pt2 = pose.pts[ pose.lines[i].second ];

			glVertex3f( pt1[0], pt1[1], pt1[2] );
			glVertex3f( pt2[0], pt2[1], pt2[2] );
		glEnd();
	}

	for( unsigned i(0); i < pose.pts.size(); ++i){
	
		const vec3f& pt = pose.pts[i];

		glPointSize(5);
		glBegin(GL_POINTS);
			glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

			glVertex3f( pt[0], pt[1], pt[2] );
		glEnd();
	}
}

void interpFrame(  nodeTransform* frame1, nodeTransform* frame2, nodeTransform*& newParent, unsigned childNum, float weight){

	nodeTransform* newNode = new nodeTransform( *frame1 );
	newNode->parent = newParent;

	newNode->R = slerp( frame1->R, frame2->R, weight);

	if( newParent != NULL ){
	
		newParent->children[ childNum ] = newNode;
	}else{
	
		newParent = newNode;
	}

	for( int i(0); i < frame1->numChildren; ++i)
	{
		interpFrame( frame1->children[i], frame2->children[i], newNode, i, weight);
	}
}

void copyFrame( nodeTransform* nT, nodeTransform*& newParent, unsigned childNum){

	nodeTransform* newNode = new nodeTransform( *nT );
	newNode->parent = newParent;

	if( newParent != NULL ){
	
		newParent->children[ childNum ] = newNode;
	}else{
	
		newParent = newNode;
	}

	for( int i(0); i < nT->numChildren; ++i)
	{
		copyFrame( nT->children[i], newNode, i);
	}
}

void drawMarker( const vec3f& v1, const vec3f& v2, float alpha1, float alpha2){

	double size = 5.0;
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/*
	glBegin( GL_POLYGON );
		glVertex3f( v[0] - size/2.0, v[1] - size/2.0, v[2]);
		glVertex3f( v[0] + size/2.0, v[1] - size/2.0, v[2]);
		glVertex3f( v[0] + size/2.0, v[1] + size/2.0, v[2]);
		glVertex3f( v[0] - size/2.0, v[1] + size/2.0, v[2]);
	glEnd();*/

	/*glPointSize(5);
	glBegin(GL_POINTS);
		glVertex3f( v1[0], v1[1], v1[2] );
	glEnd();*/

	glBegin(GL_LINES);
		glColor4f( 1.0, 0.0, 0.0, alpha1);
		glVertex3f( v1[0], v1[1], v1[2] );
		glColor4f( 1.0, 0.0, 0.0, alpha2);
		glVertex3f( v2[0], v2[1], v2[2] );
	glEnd();

	glDisable (GL_BLEND); 


}


void drawStreamer( Animation& ani, unsigned selected_stream){

	vector< vec3f > list;
	ani.getStreamerList( selected_stream, list);

	float alpha1 = 0.99;
	float alpha2 = 1.0;
	for( unsigned j(1); j < list.size(); ++j)
	{
		drawMarker( list[j - 1], list[j], alpha1, alpha2);
		alpha1 -= 0.01;
		alpha2 -= 0.01;
	}
}

void drawStreamers( Animation& ani, node* n){

	for ( unsigned i(0); i < n->numChildren; ++i){
	
		vector< vec3f > list;
		ani.getStreamerList( n->children[i], list);

		float alpha1 = 0.99;
		float alpha2 = 1.0;
		for( unsigned j(1); j < list.size(); ++j)
		{
			drawMarker( list[j - 1], list[j], alpha1, alpha2);
			alpha1 -= 0.01;
			alpha2 -= 0.01;
		}

		drawStreamers( ani, n->children[i]);
	}
}

void drawFigure( node* n ){

	float boneLength = 0.0;
	
	for ( unsigned i(0); i <  (n->numChildren); ++i){

		node* child = n->children[i];
		
		glPointSize(5);
		glBegin(GL_POINTS);
			glColor4f(0.0f, 1.0f, 0.0f, 1.f);
			glVertex3f(n->pos[0], n->pos[1], n->pos[2]);
			glColor4f(0.0f, 0.0f, 1.0f, 1.f);
			glVertex3f( child->pos[0], child->pos[1], child->pos[2]);
		glEnd();

		glBegin(GL_LINES);
			glColor4f(0.3f, 0.3f, 0.3f, 1.f);
			glVertex3f( n->pos[0], n->pos[1], n->pos[2]);
			glVertex3f( child->pos[0], child->pos[1], child->pos[2]);
		glEnd();

		float boneLength = ( n->pos - child->pos).magnitude();

		vec3f bone( 0.0f, boneLength, 0.0f );
		vec3f joint = child->pos - n->pos;

		vec3f axis = ( bone^joint).normalize();

		float theta = angle( bone, joint);

		drawBone( n->pos, axis, theta, boneLength );

		drawFigure( child );
	}

	if( (n->numChildren) == 0){
	
		float boneLength = ( n->pos - n->endPos ).magnitude();

		vec3f bone( 0.0f, boneLength, 0.0f );
		vec3f joint = n->endPos - n->pos;

		vec3f axis = ( bone^joint).normalize();

		float theta = angle( bone, joint);

		drawBone( n->pos, axis, theta, boneLength );

		glPointSize(5);
		glBegin(GL_POINTS);
			glColor4f(0.0f, 0.0f, 1.0f, 1.f);
			glVertex3f( n->endPos[0], n->endPos[1], n->endPos[2]);
		glEnd();
	}
}

void update(int value) {

	passed_time += TIMER_MS;
	
	if(quit){
		return;
	}else{
		glutTimerFunc(TIMER_MS, update, 0);
	}
	
	unsigned w = 0;
	string inputLine[ 30 ];

	if( commandMode ){
		
		system("cls");
		cout << ">>" << endl;

		bool read = true;

		while( read ){

			string tmp;
			cin >> tmp;

			if( tmp[ tmp.length() - 1] == ';'){
			
				inputLine[ w++ ] = tmp.substr(0, tmp.length() - 1);
				read = false;
			}else{
			
				inputLine[ w++ ] = tmp;
			}
		}

		if( inputLine[0] == "load" ){
		
			loadAnimation( inputLine[1], figure1);
		}else if( inputLine[0] == "splice" ){

			loadAnimation( inputLine[1], figure1);
			loadAnimation( inputLine[2], figure2);

			stringstream ss;

			ss << inputLine;
		}
 
		commandMode = false;
	}

	PerspectiveSet(gW, gH, camera->fov);

	glEnable(GL_FOG);
	GLfloat fogColor[] = {0.f, 0.f, 0.f, 1};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 0.0f);
    glFogf(GL_FOG_END, draw_dist);


	glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	figure1->figure->offset[1] = 40;
	
	if( ORBIT_MODE ){
		//camera->at[0] = figure->pos[0] /100.0;
		//camera->at[1] = figure->pos[1] /100.0;
		//camera->at[2] = figure->pos[2] /100.0;
	}

	gluLookAt(camera->pos[0],camera->pos[1],camera->pos[2],
		  camera->at[0],camera->at[1],camera->at[2],
		  camera->up[0],camera->up[1],camera->up[2]);



	glBindTexture(GL_TEXTURE_2D, checkerTex.ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) ;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable( GL_TEXTURE_2D );
	glNormal3f(0.0,1.0,0.0);
	
	float floor_color[4] = {1.0, 1.0, 1.0, 1.0};
	glColor4fv( floor_color);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, floor_color);

	glBegin(GL_POLYGON);
		
		glTexCoord2f (0.0, 0.0);
		glVertex3f(-30,0,-30);

		glTexCoord2f (0.0, 8.);
		glVertex3f(-30,0, 30);

		glTexCoord2f (8., 8.);
		glVertex3f(30, 0, 30);

		glTexCoord2f (8., 0.0);
		glVertex3f(30, 0, -30);
	glEnd();

	glDisable( GL_TEXTURE_2D );

	glPushMatrix();
	
	glScalef(.01, .01, .01);

	static double angle = 0.0f;
	angle++;
	if(angle >= 360)	angle = 0;

	if(play){
		
		figure1->nextFrame();
	}

	pose->lines.clear();
	pose->pts.clear();
	pose->vlines.clear();

	figure1->computePos( *pose, figure1->figure->startingPos[ figure1->currentFrame ] );
	
	//if(play){
	figure1->addStreamers();
	//}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawFigure( figure1->figure );
	//drawStreamers( *figure1, figure1->figure );
	drawStreamer( *figure1, selected_stream );
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPopMatrix();

	draw2DModeON();

	draw2DModeOFF();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0.0, (GLdouble)gW, 0.0, (GLdouble)gH, .1, draw_dist);
	glViewport(0, 0, gW, gH);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_FOG);

	ui->draw();

	glutSwapBuffers();
}

string genTabs( unsigned numTabs){

	string tmp;

	for( int i(0); i < numTabs; ++i){
	
		tmp += "\t";
	}

	return tmp;
}

void writeJoint( fstream& outFile, node* node, unsigned tabLevel){
	
	outFile << genTabs(tabLevel) << "OFFSET " << (node->offset[0]) << " " << (node->offset[1]) << " " << (node->offset[2]) << "\n";
	
	outFile << genTabs(tabLevel) << "CHANNELS " << node->DOF << " ";

	for(int i = 0; i < node->DOF; ++i){
		
		outFile << (node->channels[i]) << " ";
	}
	outFile << genTabs(tabLevel) << "\n";

	for(int i = 0; i < node->numChildren; ++i){

		outFile << genTabs(tabLevel) << "JOINT " << node->children[i]->name << "\n" <<  genTabs(tabLevel) << "{\n";
		
		writeJoint( outFile, (node->children[i]), tabLevel + 1);

		outFile << genTabs(tabLevel) << "}\n";
	}

	if( node->numChildren == 0){ //endsite
		
		outFile << genTabs(tabLevel) << "End Site\n" <<  genTabs(tabLevel) << "{\n";
		outFile << genTabs(tabLevel + 1) << "OFFSET " << (node->endSite[0]) << " " << (node->endSite[1]) << " " << (node->endSite[2]) << "\n";
		outFile << genTabs(tabLevel) << "}\n";
	}
}

void writeMotions( fstream& outFile, nodeTransform& nT){

	for( int i(0); i < 3; ++i){
		outFile << (nT.rotation[i]) << " ";
	}

	for( int i(0); i < nT.numChildren; ++i){
	
		writeMotions( outFile, *nT.children[i]);
	}
}


void writeBVH( const string& fileName, Animation* ani){

	fstream outFile;
	outFile.open ( fileName, fstream::in | fstream::out);

	outFile << "HIERARCHY\n";
	outFile << "ROOT " << ani->figure->name << "\n{\n";

	writeJoint( outFile, ani->figure, 1);

	outFile << "}\n";

	outFile << "MOTION\n";
	outFile << "Frames: " << (ani->numFrames) << "\n";
	outFile << "Frame Time: " << ani->frameRate << "\n";

	for( int k(0); k < ani->motionVector.size(); ++k){

		outFile << ani->figure->startingPos[k][0] << " " << ani->figure->startingPos[k][1] << " " << ani->figure->startingPos[k][2] << " ";
		writeMotions( outFile, *ani->motionVector[k]);
		outFile << "\n";
	}
}


void main(int argc, char **argv)
{
	//seed the random number generator to give different results each time
	srand(time(0));

	ww = 640;
	wh = 480;

	gW = 640;
	gH = 480;

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB |GLUT_DEPTH);
    glutInitWindowSize(gW, gH);
    glutCreateWindow("Mocap Viewer");

	init2();

	glutMainLoop();

	
	delete pose;
	delete ui;
	delete camera;
	delete figure1;
	delete figure2;
}
