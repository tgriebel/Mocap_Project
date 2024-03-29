#include "Frame2D.h"
#include <vector>
using namespace std;

void Frame2D::createFrameDimensions(double xi, double yi, double zi, double widthi, double lengthi, double heighti){
	setX(xi); setY(yi); setZ(zi);
	setWidth(widthi); setLength(lengthi); setHeight(heighti);
}
void Frame2D::mapTextureDimensions(double xLowerbound, double yLowerbound, double xUpperbound, double yUpperbound){
	texX1 = xLowerbound;
	texX2 = xUpperbound;

	texY1 = yLowerbound;
	texY2 = yUpperbound;
}
inline void Frame2D::createFrameDimensions(double pos[3], double dim[3]){//wrapper function
	createFrameDimensions(pos[0], pos[1], pos[2], dim[0], dim[1], dim[2]);
}
inline void Frame2D::mapTextureDimensions(double pos[2], double dim[2]){//wrapper function
	mapTextureDimensions(pos[0], pos[1], dim[0], dim[1]);
}
void Frame2D::draw(){
	glBindTexture(GL_TEXTURE_2D, tTag->ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//glPushAttrib();//push color
	glColor4fv(color);

	glBegin(GL_POLYGON);

		glTexCoord2f(texX1,texY1);
		glVertex3f(getX()-getWidth()/2.0,getY(),getZ());

		glTexCoord2f(texX2,texY1);
		glVertex3f(getX()+getWidth()/2.0,getY(),getZ());

		glTexCoord2f(texX1,texY2);
		glVertex3f(getX()+getWidth()/2.0,getY()+getHeight(),getZ());

		glTexCoord2f(texX1,texY2);
		glVertex3f(getX()-getWidth()/2.0,getY()+getHeight(),getZ());


	glEnd();

	//glPopAttrib();//pops color

}