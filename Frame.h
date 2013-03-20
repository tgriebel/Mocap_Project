//////////////////////////////////////////////////////////////////////////////
//Name: Frame Class															//
//Description: Abstract Class. Child can specify the contents of the frame	//
//				A frame child might be a 3D or 2D figure					//
//Author: Thomas Griebel													//
//Date Last Modified: 12/23/2010											//
//Date Started: 12/23/2010													//
//////////////////////////////////////////////////////////////////////////////

#ifndef ___FRAME___
#define ___FRAME___

class Frame{

private: 
	double x;
	double y;
	double z;

	double width;
	double length;
	double height;
public:
	Frame():x(0.0),y(0.0),z(0.0), width(1.0), length(1.0), height(1.0){}
	virtual ~Frame(){};

	virtual void draw() = 0;

	inline double getX()						{ return x;}
	inline double getY()						{ return y;}
	inline double getZ()						{ return z;}

	inline void setX(double setX)			{ x = setX;}
	inline void setY(double setY)			{ y = setY;}
	inline void setZ(double setZ)			{ z = setZ;}

	inline double getWidth()					{ return width;}
	inline double getLength()				{ return length;}
	inline double getHeight()				{ return height;}

	inline void setWidth(double setWidth)	{ width = setWidth;}
	inline void setLength(double setLength)	{ length = setLength;}
	inline void setHeight(double setHeight)	{ height = setHeight;}
};


#endif