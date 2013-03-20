#include "FreeCamera.h"


void FreeCamera::dolly(double speed){

	vec3d dv = (at - pos)*speed;
	pos += dv;
	at += dv;
}

void FreeCamera::dollyCamera(double speed){

	vec3d dv = (at - pos)*speed;
	pos += dv;
}

void FreeCamera::dollyTarget(double speed){

	vec3d dv = (at - pos)*speed;
	at += dv;
}

void FreeCamera::truckY(double speed){

	vec3d dv = up*speed;
	pos += dv;
	at += dv;
}
void FreeCamera::truckX(double speed){

	side = (at - pos).normalize()^up;

	vec3d dv = side*speed;
	pos += dv;
	at += dv;
}

void FreeCamera::orbitX(double theta){

	double phi = degreeToRadian(theta);

	vec3d v =  pos - at;

	rotate(phi, up, v);

	pos = at + v;
}

void FreeCamera::orbitY(double theta){

	double phi = degreeToRadian(theta);

	vec3d p = pos - at;

	vec3d v = p.normalize()^up;

	rotate(phi, v, p);
	pos = at + p;

	rotate(phi, v, up);
}

void FreeCamera::panX(double theta){

	double phi = degreeToRadian(theta);

	vec3d v = at - pos;

	rotate(phi, up, v);

	at = v + pos;
}

void FreeCamera::panY(double theta){

	double phi = degreeToRadian(theta);

	vec3d p = at - pos;

	vec3d v = p.normalize()^up;

	rotate(phi, v, p);
	at = p + pos;

	rotate(phi, v, up);
}

void FreeCamera::roll(double theta){

	double phi = degreeToRadian(theta);

	vec3d v = at - pos;

	rotate(phi, v, up);
}

void FreeCamera::rotate(double theta, vec3d& axis, vec3d& point){

		axis = axis.normalize();

		Quaternion<double> V, R, Rp;

		V.x = point[0];
		V.y = point[1];
		V.z = point[2];
		V.w = 0.0f;

		R.x = axis[0] * sin(theta/2.0f);
		R.y = axis[1] * sin(theta/2.0f);
		R.z = axis[2] * sin(theta/2.0f);
		R.w = cos(theta/2.0f);

		Rp = R.conjugate();
		V = mult(mult(R, V), Rp);

		point[0] = V.x;
		point[1] = V.y;
		point[2] = V.z;
}
