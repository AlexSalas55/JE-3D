/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	Here we define all the mathematical classes like Vector3, Matrix44 and some extra useful geometrical functions
*/

#pragma once

#include <vector>
#include <cmath>
#include <stdlib.h>
#include <ostream>


#ifndef PI
	#define PI 3.14159265359f
#endif
#define DEG2RAD 0.0174532925f
#define RAD2DEG 57.295779513f

//more standard type definition
typedef char int8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short int16;
typedef int int32;
typedef unsigned int uint32;

//ideas to use them https://www.youtube.com/watch?v=R6UB7mVO3fY
inline float clamp(float v, float a = 0.0f, float b = 1.0f) { return v < a ? a : (v > b ? b : v); }
inline float lerp(float a, float b, float v ) { return a*(1.0f-v) + b*v; }
inline float invlerp(float a, float b, float v ) { return (v-a)/(b-a); }
inline float remap(float iMin, float iMax, float oMin, float oMax, float v ) { return lerp( oMin,oMax, invlerp(iMin,iMax,v)); }

class Vector2
{
public:
	union
	{
		struct { float x,y; };
		float value[2];
	};

	Vector2() { x = y = 0.0f; }
	Vector2(float x, float y) { this->x = x; this->y = y; }
	Vector2(float v) { this->x = v; this->y = v; }

	double length() { return sqrt(x*x + y*y); }
	double length() const { return sqrt(x*x + y*y); }

	float dot( const Vector2& v );
	float perpdot( const Vector2& v );

	void set(float x, float y) { this->x = x; this->y = y; }

	Vector2& normalize() { *this *= (float)length(); return *this; }

	float distance(const Vector2& v);
	void random(float range);
	void parseFromText(const char* text);

	void operator *= (float v) { x*=v; y*=v; }
};

Vector2 operator * (const Vector2& a, float v);
Vector2 operator + (const Vector2& a, const Vector2& b);
Vector2 operator - (const Vector2& a, const Vector2& b);
Vector2 operator / (const Vector2& a, const Vector2& b);

Vector2 normalize(Vector2 n);
inline Vector2 lerp(const Vector2& a, const Vector2& b, float v) { return a*(1.0f - v) + b*v; }

class Vector3u
{
public:
	union
	{
		struct { unsigned int x;
				 unsigned int y;
				 unsigned int z; };
		unsigned int v[3];
	};
	Vector3u() { x = y = z = 0; }
	Vector3u(unsigned int x, unsigned int y, unsigned int z) { this->x = x; this->y = y; this->z = z; }
	void set(unsigned int x, unsigned int y, unsigned int z) { this->x = x; this->y = y; this->z = z; }
};

//*********************************

class Vector3
{
public:

	static Vector3 UP;

	//float x, y, z;




	union
	{
		struct { float x,y,z; };
		float v[3];
	};

	Vector3() { x = y = z = 0.0f; }
	Vector3(float x, float y, float z) { this->x = x; this->y = y; this->z = z;	}
	Vector3(float v) { this->x = v; this->y = v; this->z = v; }

	// Overload the << operator
	friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
		os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return os;
	}

	double length();
	double length() const;

	void set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
	void set(float v) { this->x = v; this->y = v; this->z = v; }

	Vector2 xy() { return Vector2(x, y); }

	void setMin(const Vector3 & v);
	void setMax(const Vector3 & v);

	Vector3& normalize();
	void random(float range);
	void random(Vector3 range);

	float distance(const Vector3& v) const;

	Vector3 cross( const Vector3& v ) const;
	float dot( const Vector3& v ) const;

	void parseFromText(const char* text, const char separator);

	float& operator [] (int n) { return v[n]; }
	void operator *= (float v) { x *= v; y *= v; z *= v; }
	void operator += (const Vector3& v) { x += v.x; y += v.y; z += v.z; }
	void operator -= (const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; }
};

Vector3 normalize(Vector3 n);
float dot( const Vector3& a, const Vector3& b);
Vector3 cross(const Vector3&a, const Vector3& b);
Vector3 lerp(const Vector3& a, const Vector3& b, float v);

inline Vector3 operator + (const Vector3& a, const Vector3& b) { return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3 operator - (const Vector3& a, const Vector3& b) { return Vector3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3 operator - (const Vector3& a) { return Vector3(-a.x, -a.y, -a.z); }
inline Vector3 operator * (const Vector3& a, const Vector3& b) { return Vector3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline Vector3 operator * (const Vector3& a, float v) { return Vector3(a.x * v, a.y * v, a.z * v); }
inline Vector3 operator * (float v, const Vector3& a) { return Vector3(a.x * v, a.y * v, a.z * v); }
inline Vector3 operator / (const Vector3& a, float v) { return Vector3(a.x / v, a.y / v, a.z / v); }
inline Vector3 operator / (float v, const Vector3& a) { return Vector3(a.x / v, a.y / v, a.z / v); }

class Vector4
{
public:
	union
	{
		struct { float x,y,z,w; };
		float v[4];
		struct { Vector3 xyz; } sV4Data;
	};

	Vector4() { x = y = z = w = 0.0f; }
	Vector4(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
	Vector4(float v) { this->x = v; this->y = v; this->z = v; this->w = v; }
	Vector4(const Vector3& v, float w) { x = v.x; y = v.y; z = v.z; this->w = w; }
	Vector4(const float* v) { x = v[0]; x = v[1]; x = v[2]; x = v[3]; }
    void set(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
	void set(float v) { this->x = v; this->y = v; this->z = v; this->w = v; }

	Vector3 xyz() { return Vector3(x, y, z); }

	static Vector4 WHITE;
	static Vector4 RED;
	static Vector4 GREEN;
	static Vector4 BLUE;
};

inline Vector4 operator * (const Vector4& a, float v) { return Vector4(a.x * v, a.y * v, a.z * v, a.w * v); }
inline Vector4 operator + (const Vector4& a, const Vector4& b) { return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline Vector4 lerp(const Vector4& a, const Vector4& b, float v) { return a*(1.0f - v) + b*v; }

//can be used to store colors
class Vector4ub
{
public:
	union
	{
		struct {
			uint8 x;
			uint8 y;
			uint8 z;
			uint8 w;
		};
		struct {
			uint8 r;
			uint8 g;
			uint8 b;
			uint8 a;
		};
		uint8 v[4];
	};
	Vector4ub() { x = y = z = 0; }
	Vector4ub(uint8 x, uint8 y, uint8 z, uint8 w = 0) { this->x = x; this->y = y; this->z = z; this->w = w; }
	void set(uint8 x, uint8 y, uint8 z, uint8 w = 0) { this->x = x; this->y = y; this->z = z; this->w = w; }
	Vector4ub operator = (const Vector4& a) { x = (uint8)a.x; y = (uint8)a.y; z = (uint8)a.z; w = (uint8)a.w; return *this; }
	Vector4 toVector4() { return Vector4(x, y, z, w); }
};

inline Vector4ub operator + (const Vector4ub& a, const Vector4ub& b) { return Vector4ub(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w ); }
inline Vector4ub operator * (const Vector4ub& a, float v) { return Vector4ub((uint8)(a.x * v), (uint8)(a.y * v), (uint8)(a.z * v), (uint8)(a.w * v)); }
inline bool operator == (const Vector4ub& a, const Vector4ub& b) { return a.x == b.x && a.y == b.y && a.z == b.z; } //only colors, no alpha
inline Vector4ub lerp(const Vector4ub& a, const Vector4ub& b, float v) { return a*(1.0f - v) + b*v; }

typedef Vector4ub Color;

class Quaternion;

//****************************
//Matrix44 class
class Matrix44
{
	public:
		static const Matrix44 IDENTITY;

		//This matrix works in 
		union { //allows to access the same var using different ways
			struct
			{
				float        _11, _12, _13, _14;
				float        _21, _22, _23, _24;
				float        _31, _32, _33, _34;
				float        _41, _42, _43, _44;
			};
			float M[4][4]; //[row][column]
			float m[16];
		};

		Matrix44();
		Matrix44(const float* v);

		void set(); //multiply with opengl matrix
		void load(); //load in opengl matrix
		void clear();
		void setIdentity();
		void transpose();
		//void normalizeAxis();

		//get base vectors
		Vector3 rightVector() { return Vector3(m[0],m[1],m[2]); }
		Vector3 topVector() { return Vector3(m[4],m[5],m[6]); }
		Vector3 frontVector() { return Vector3(m[8],m[9],m[10]); }

		bool inverse();
		void setUpAndOrthonormalize(Vector3 up);
		void setFrontAndOrthonormalize(Vector3 front);

		Matrix44 getRotationOnly(); //used when having scale

		//rotate only
		Vector3 rotateVector( const Vector3& v) const;

		//transform using local coordinates
		void translate(float x, float y, float z);
		void translate(const Vector3& delta);
		void rotate( float angle_in_rad, const Vector3& axis  );
		void scale(float x, float y, float z);
		void scale(const Vector3& delta);

		//transform using global coordinates
		void translateGlobal(float x, float y, float z);
		void translateGlobal(const Vector3& v);
		void rotateGlobal( float angle_in_rad, const Vector3& axis  );

		//create a transformation matrix from scratch
		void setTranslation(float x, float y, float z);
		void setTranslation(const Vector3& new_translation);
		void setRotation( float angle_in_rad, const Vector3& axis );
		void setScale(float x, float y, float z);
		void decompose(Vector3& translation, Quaternion& rotation, Vector3& scale);
		void compose(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);

		Vector3 getTranslation() const;

		bool getXYZ(float* euler) const;

		float getYawRotationToAimTo(const Vector3& position);

		void lookAt(Vector3& eye, Vector3& center, Vector3& up);
		void perspective(float fov, float aspect, float near_plane, float far_plane);
		void ortho(float left, float right, float bottom, float top, float near_plane, float far_plane);

		Vector3 project(const Vector3& v);

		void toQuaternion(Quaternion&) const;

		//old fixed pipeline (do not used if possible)
		void multGL();
		void loadGL();

		Matrix44 operator * (const Matrix44& matrix) const;
};

//Operators, they are our friends
//Matrix44 operator * ( const Matrix44& a, const Matrix44& b );
Vector3 operator * (const Matrix44& matrix, const Vector3& v);
Vector4 operator * (const Matrix44& matrix, const Vector4& v); 


class Quaternion
{
public:

	union
	{
		struct { float x; float y; float z; float w; };
		float q[4];
	};

	static Quaternion FromMatrix(const Matrix44& matrix) {
		Quaternion q;
		q.setFromMatrix(matrix);
		return q;
	}

	void setFromMatrix(const Matrix44& matrix) {
		float trace = matrix.m[0] + matrix.m[5] + matrix.m[10];
		if (trace > 0.0f) {
			float s = sqrt(trace + 1.0f) * 2.0f;
			w = 0.25f * s;
			x = (matrix.m[9] - matrix.m[6]) / s;
			y = (matrix.m[2] - matrix.m[8]) / s;
			z = (matrix.m[4] - matrix.m[1]) / s;
		}
		else if ((matrix.m[0] > matrix.m[5]) && (matrix.m[0] > matrix.m[10])) {
			float s = sqrt(1.0f + matrix.m[0] - matrix.m[5] - matrix.m[10]) * 2.0f;
			w = (matrix.m[9] - matrix.m[6]) / s;
			x = 0.25f * s;
			y = (matrix.m[1] + matrix.m[4]) / s;
			z = (matrix.m[2] + matrix.m[8]) / s;
		}
		else if (matrix.m[5] > matrix.m[10]) {
			float s = sqrt(1.0f + matrix.m[5] - matrix.m[0] - matrix.m[10]) * 2.0f;
			w = (matrix.m[2] - matrix.m[8]) / s;
			x = (matrix.m[1] + matrix.m[4]) / s;
		}
	}

public:
	Quaternion();
	Quaternion(const float* q);
	Quaternion(const Quaternion& q);
	Quaternion(const float X, const float Y, const float Z, const float W);
	Quaternion(const Vector3& axis, float angle);

	void identity();
	Quaternion invert() const;
	Quaternion conjugate() const;

	void set(const float X, const float Y, const float Z, const float W);
	void slerp(const Quaternion& b, float t);
	void slerp(const Quaternion& q2, float t, Quaternion &q3) const;

	void lerp(const Quaternion& b, float t);
	void lerp(const Quaternion& q2, float t, Quaternion &q3) const;

public:
	void setAxisAngle(const Vector3& axis, const float angle);
	void setAxisAngle(float x, float y, float z, float angle);
	void getAxisAngle(Vector3 &v, float &angle) const;

	Vector3 rotate(const Vector3& v) const;

	void operator*=(const Vector3& v);
	void operator *= (const Quaternion &q);
	void operator += (const Quaternion &q);

	friend Quaternion operator + (const Quaternion &q1, const Quaternion& q2);
	friend Quaternion operator * (const Quaternion &q1, const Quaternion& q2);

	friend Quaternion operator * (const Quaternion &q, const Vector3& v);

	friend Quaternion operator * (float f, const Quaternion &q);
	friend Quaternion operator * (const Quaternion &q, float f);

	Quaternion& operator -();


	friend bool operator==(const Quaternion& q1, const Quaternion& q2);
	friend bool operator!=(const Quaternion& q1, const Quaternion& q2);

	void operator *= (float f);

	void computeMinimumRotation(const Vector3& rotateFrom, const Vector3& rotateTo);

	void normalize();
	float squaredLength() const;
	float length() const;
	void toMatrix(Matrix44 &) const;

	void toEulerAngles(Vector3 &euler) const;

	float& operator[] (unsigned int i) { return q[i]; }
};

float DotProduct(const Quaternion &q1, const Quaternion &q2);
Quaternion Qlerp(const Quaternion &q1, const Quaternion &q2, float t);
Quaternion Qslerp(const Quaternion &q1, const Quaternion &q2, float t);
// Quaternion Qsquad(const Quaternion &q1, const Quaternion &q2, const Quaternion &a, const Quaternion &b, float t);
// Quaternion Qsquad(const Quaternion &q1, const Quaternion &q2, const Quaternion &a, float t);
// Quaternion Qspline(const Quaternion &q1, const Quaternion &q2, const Quaternion &q3);
// Quaternion QslerpNoInvert(const Quaternion &q1, const Quaternion &q2, float t);
Quaternion Qexp(const Quaternion &q);
Quaternion Qlog(const Quaternion &q);
Quaternion SimpleRotation(const Vector3 &a, const Vector3 &b);

class BoundingBox
{
public:
	Vector3 center;
	Vector3 halfsize;
	BoundingBox() {}
	BoundingBox(Vector3 center, Vector3 halfsize) { this->center = center; this->halfsize = halfsize; };
};

//applies a transform to a AABB so it is 
BoundingBox transformBoundingBox(const Matrix44 m, const BoundingBox& box);

enum {
	CLIP_OUTSIDE = 0,
	CLIP_OVERLAP,
	CLIP_INSIDE
};

float signedDistanceToPlane(const Vector4& plane, const Vector3& point);
int planeBoxOverlap( const Vector4& plane, const Vector3& center, const Vector3& halfsize );
float ComputeSignedAngle( Vector2 a, Vector2 b); //returns the angle between both vectors in radians
inline float ease(float f) { return f*f*f*(f*(f*6.0f - 15.0f) + 10.0f); }
Vector3 RayPlaneCollision( const Vector3& plane_pos, const Vector3& plane_normal, const Vector3& ray_origin, const Vector3& ray_dir );
bool RaySphereCollision(const Vector3& center, const float& radius, const Vector3& ray_origin, const Vector3& ray_dir, Vector3& coll);
Vector3 reflect(const Vector3& I, const Vector3& N);

//value between 0 and 1
inline float random(float range = 1.0f, float offset = 0.0f) { return static_cast<float>(((rand() % 10000) / (10000.0))) * range + offset; }


typedef Vector3 vec2;
typedef Vector3 vec3;
typedef Vector4 vec4;
typedef Matrix44 mat4;
typedef Quaternion quat;