//////////////////////////////////////////////////////////////////////////////
//Name: 2D Frame Class														//
//Description: Created from cropping a sprite sheet for a given 2D frame	//
//				Child class of abstract Frame class							//
//Author: Thomas Griebel													//
//Date Last Modified: 05/28/2011											//
//Date Started: 12/22/2010													//
//////////////////////////////////////////////////////////////////////////////

#ifndef ___FRAME2D___
#define ___FRAME2D___

#include "Frame.h"
#include "TextureStructs.h"

#include <iostream>
using namespace std;// for debugging purposes only

#include <gl\glut.h>

class Frame2D: public Frame{

private:
	float color[4];
	double texX1;//texture x
	double texY1;
	double texX2;//texture x endpoint
	double texY2;
	TextureID* tTag;
public:
	using Frame::draw; 

	Frame2D(TextureID* const tTagSet)
		: texX1(0.0), texY1(0.0), texX2(0.0), texY2(0.0), tTag(tTagSet)
	{
		color[0] = color[1] = color[2] = color[3] = 1.0;
	}
	Frame2D(TextureID* const tTagSet, double textureLowerX, double textureLowerY, double textureUpperX, double textureUpperY)
		:	texX1(textureLowerX), 
			texY1(textureLowerY), 
			texX2(textureUpperX), 
			texY2(textureUpperY),
			tTag(tTagSet)
	{
		color[0] = color[1] = color[2] = color[3] = 1.0;
	}

	virtual ~Frame2D(){};

	void createFrameDimensions(double, double, double, double, double, double);
	inline void createFrameDimensions(double pos[3], double dim[3]);//forwarding function

	void mapTextureDimensions(double, double, double, double);
	inline void mapTextureDimensions(double pos[2], double dim[2]);

	virtual void draw();

};


#endif