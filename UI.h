
#ifndef ___ANIMATION_CLASSES___
#define ___ANIMATION_CLASSES___
#include "TextureStructs.h"
#include "menu.h"

#include <sstream>
#include <string>
using namespace std;

struct Button{

	int frame;
	bool* global_control;
	double x, y, h, w;
	TextureID frames[2];

	void (*click)(void);
	void (*unclick)(void);

	void draw(){
	
		glEnable(GL_TEXTURE_2D);

		if(!*global_control)
			glBindTexture(GL_TEXTURE_2D, frames[0].ID);
		else
			glBindTexture(GL_TEXTURE_2D, frames[1].ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		float color[4] = {1.0, 1.0, 1.0,1.0};
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glColor4fv(color);
		glBegin(GL_POLYGON);
			
			glTexCoord2f(0.0,0.0);
			glVertex3f(x, y, -9);

			glTexCoord2f(1.0,0.0);
			glVertex3f(x + w, y, -9);

			glTexCoord2f(1.0, 1.0);
			glVertex3f(x + w, y + h, -9);

			glTexCoord2f(0.0, 1.0);
			glVertex3f(x, y + h, -9);

		glEnd();

		glDisable(GL_TEXTURE_2D);
	}
};

struct UI{

private:
	int x;
	int y;
	int height;
	int width;
	TextureID tTag;

	double sliderX;
	double sliderY;
	double sliderW;
	double sliderH;

	double railX;
	double railY;
	double railW;
	double railH;

	bool mouseMoved;
public:
	Button buttons[10];
	int numButs;
	unsigned* totalFrames;
	unsigned* currentFrame;
	unsigned* fps;
	
	UI(int hi, int wi, const TextureID& tagSet)
	: 
		numButs(0), 
		totalFrames(0), currentFrame(0), 
		sliderX(154.0f), sliderY(40.0f), sliderW(6.0f), sliderH(15.0),
		railX(154.0f), railY( 40 ), railW( 410.0f - 154.0f), railH( 4.0),
		height(hi), width(wi), 
		tTag(tagSet),
		mouseMoved(false)
	{}

	void resize(int ws, int hs){
		height = hs;
		width  = ws;
	}

	void mouseMove(int mx, int my){

		if(mx >= railX && mx <= railX + railW &&
			my >= railY && my <= railY + sliderH)
		{
			sliderX = ( ((double)(mx-railX)) / ((double)railW)) * (railW) + railX;
			(*currentFrame) = (mx-railX);
			mouseMoved = true;
		}else{
			mouseMoved = false;
		}
	}

	void mouseClick(int mx, int my){

		static int oX, oY;
		
		for(int i(0); i < numButs; ++i){

			if(mx >= buttons[i].x && mx <= buttons[i].x + buttons[i].w &&
				my >= buttons[i].y && my <= buttons[i].y + buttons[i].h)
			{
				buttons[i].click();
			}
		}

		oX = mx;
		oY = my;
	}

	void draw(){

		glPushMatrix();
		glPushAttrib( GL_COLOR_BUFFER_BIT );

		stringstream ss;
		ss << *fps;

		string text = ss.str() + "fps";

		glTranslatef (560, 440, -8.0f);
		glScalef(0.25, 0.25, 0.25);

		glColor3f( 1.0, 0.0, 0.0);
		for (unsigned i(0); i < text.length(); ++i)
			glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);

		glPopAttrib();
		glPopMatrix();

		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, tTag.ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		float color[4] = {1.0, 1.0, 1.0,1.0};
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glColor4f(1.0,1.0,1.0,1.0);
		glBegin(GL_POLYGON);
			
			glTexCoord2f(0.0,0.0);
			glVertex3f(0,0,-10);

			glTexCoord2f(1.0,0.0);
			glVertex3f(width,0,-10);

			glTexCoord2f(1.0,1.0);
			glVertex3f(width, height/6.0,-10);

			glTexCoord2f(0.0,1.0);
			glVertex3f(0,height/6.0,-10);

		glEnd();

		glDisable(GL_TEXTURE_2D);

		GLfloat dot_color[4] = {1.0, 0.0, 0.0,0.7};
		glColor4fv(dot_color);
		int size = 4;
		glBegin(GL_POLYGON);
			
			glTexCoord2f(0.0,0.0);
			glVertex3f(width/2. - size,height/2. - size,-10);

			glTexCoord2f(1.0,0.0);
			glVertex3f(width/2. +size,height/2. - size,-10);

			glTexCoord2f(1.0,1.0);
			glVertex3f(width/2. + size, height/2. + size,-10);

			glTexCoord2f(0.0,1.0);
			glVertex3f(width/2. - size,height/2. + size,-10);

		glEnd();

		if(!mouseMoved){
			sliderX = ((double)*currentFrame / (double)(*totalFrames - 1)) * (410.0f - 154.0f) + 154.0f;
		}

		glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f( railX, railY, -9);
			glVertex3f( railX + railW, railY, -9);
			
			glVertex3f( railX, railY + railH, -9);
			glVertex3f( railX + railW, railY + railH, -9);
		glEnd();

		glBegin(GL_POLYGON);
			glVertex3f( sliderX, sliderY, -9);
			glVertex3f( sliderX + sliderW, sliderY, -9);
			glVertex3f( sliderX + sliderW, sliderY + sliderH, -9);
			glVertex3f( sliderX, sliderY + sliderH, -9);
		glEnd();

		for(int i(0); i < numButs; i++){
			buttons[i].draw();
		}

		glPushMatrix();
		glPushAttrib( GL_COLOR_BUFFER_BIT );

		ss = stringstream();
		ss << *currentFrame;

		text.clear();
		text = ss.str() + "/";

		ss = stringstream();

		ss << (*totalFrames - 1);

		text += ss.str();

		glTranslatef ( 150, 10, -8.0f);
		glScalef(0.15, 0.15, 0.15);

		glColor3f( 1.0, 1.0, 1.0);
		for (unsigned i(0); i < text.length(); ++i)
			glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);

		glPopAttrib();
		glPopMatrix();
	}
};
#endif