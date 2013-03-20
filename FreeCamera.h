#ifndef __CAMERA3D_FREE__
#define __CAMERA3D_FREE__

#include<cmath>
#include<iostream>
using namespace std;
#define PI (4*atan(1.0))

#include "Quaternion.h"
#include "MVector.h"

class FreeCamera{

protected:

	//TO DO: should be utilities!
	inline void normalizeDegree(double &degree){ if(degree>=360) degree = 0;}
	inline double degreeToRadian(double degree) { return degree*(PI/180.0f);}
	inline double radianToDegree(double radian) { return radian*(180.0/PI);}

	void rotate(double theta, vec3d& axis, vec3d& point);
	
public:

	double fov;
	vec3d at;
	vec3d up;
	vec3d pos;
	vec3d side;

	FreeCamera()
	: 
		at(0.0f, 0.0f, 1.0f), 
		up(0.0f, 1.0f, 0.0f), 
		pos(0.0f, 0.0f, 0.0f)
	{ 
		fov = 78;

		side = (at - pos)^up;
	}
	FreeCamera(double posx, double posy, double posz)
	:
		at(posx, posy, posz - 1.0f), 
		up(0.0f, 1.0f, 0.0f), 
		pos(posx, posy, posz)
	{
		fov = 78;

		side = (at - pos)^up;
	}

	virtual ~FreeCamera(){}

	void dolly(double speed);//x
	void dollyTarget(double speed);//x
	void dollyCamera(double speed);//x
	void truckY(double speed);//x
	void truckX(double speed);//x

	void panX(double theta);//x
	void panY(double theta);//x

	void orbitX(double theta);//x
	void orbitY(double theta);//x

	void roll(double theta);//x
};

#endif