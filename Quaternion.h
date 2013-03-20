#ifndef ___QUATERNION___
#define ___QUATERNION___

#pragma once

#include <cmath>
#include "MVector.h"

#define PI (4.0*atan(1.0))

template <typename T>
struct Quaternion{

	T x, y, z, w;

	Quaternion( T theta, vec3d& axis){
		
		T phi = theta * ( (T)PI / (T)180.0);

		vec3d ax = axis.normalize();

		x = ax[0] * sin( phi / (T)2.0);
		y = ax[1] * sin( phi / (T)2.0);
		z = ax[2] * sin( phi / (T)2.0);
		w =			cos( phi / (T)2.0);
	}

	Quaternion( const vec3d& vec): w(0.0){
		x = vec[0];
		y = vec[1];
		z = vec[2];
	}

	Quaternion(): x(0.0), y(0.0), z(0.0), w(0.0) {}

	T length() const;
	Quaternion<T> normalize() const;
	Quaternion<T> conjugate();
};

template <typename T>
T Quaternion<T>::length() const
{  
	return sqrt(x * x + y * y + z * z + w * w); 
} 

template <typename T>
Quaternion<T> Quaternion<T>::normalize() const
{   
	T L = length();   

	Quaternion<T> q = *this;
	q.x /= L;   
	q.y /= L;   
	q.z /= L;   
	q.w /= L;
	return q;
} 

template <typename T>
Quaternion<T> Quaternion<T>::conjugate()
{
	Quaternion<T> q = *this;
	q.x = -x;   
	q.y = -y;   
	q.z = -z;
	return q;
}

template <typename T>
Quaternion<T> mult(const Quaternion<T>& A, const Quaternion<T>& B) {
	Quaternion<T> C;  
	C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;   
	C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;   
	C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;   
	C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;   
	return C;
}

template <typename T>
Quaternion<T> operator*(const Quaternion<T>& A, const Quaternion<T>& B) {
	Quaternion<T> C;  
	C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;   
	C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;   
	C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;   
	C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;   
	return C;
}

template <typename T>
void rotate( Quaternion<T>& Q, vec3d& P)
{
	Quaternion<T> V( P );

	V = mult(mult(Q, V), Q.conjugate());

	P[0] = V.x;
	P[1] = V.y;
	P[2] = V.z;
}

template <typename T>
void rotate(T theta, vec3d& axis, vec3d& P){

	Quaternion<T> V(P);
	Quaternion<T> R(theta, axis);

	V = R*V*R.conjugate();

	P[0] = V.x;
	P[1] = V.y;
	P[2] = V.z;
}

#endif