#if !defined FILE_UTIL_VECTOR_H
#define FILE_UTIL_VECTOR_H

// Misc C-runtime library headers
#include "STDIO.H"
#include "STDLIB.H"
#include "MATH.H"

// Header file containing definition of globalvars_t and entvars_t
typedef int	func_t;					//
typedef int	string_t;				// from engine's pr_comp.h;
typedef float vec_t;				// needed before including progdefs.h

//=========================================================
// 2DVector - used for many pathfinding and many other 
// operations that are treated as planar rather than 3d.
//=========================================================
class Vector2D
{
public:
	inline Vector2D(void)									{ }
	inline Vector2D(float X, float Y)						{ x = X; y = Y; }
	inline Vector2D operator+(const Vector2D& v)	const	{ return Vector2D(x+v.x, y+v.y);	}
	inline Vector2D operator-(const Vector2D& v)	const	{ return Vector2D(x-v.x, y-v.y);	}
	inline Vector2D operator*(float fl)				const	{ return Vector2D(x*fl, y*fl);	}
	inline Vector2D operator/(float fl)				const	{ return Vector2D(x/fl, y/fl);	}
	
	inline float Length(void)						const	{ return (float)sqrt(x*x + y*y );		}

	inline Vector2D Normalize ( void ) const {
		Vector2D vec2;

		float flLen = Length();
		if ( flLen == 0 )
		{
			return Vector2D( (float)0, (float)0 );
		}
		else
		{
			flLen = 1 / flLen;
			return Vector2D( x * flLen, y * flLen );
		}
	}

	inline float DotProduct(const Vector2D& v) { return(v.x * x + v.y * y); }

	vec_t	x, y;
};

#define M_PI 3.14159265358979323846

inline float POW(float x) { return x * x; }

//=========================================================
// 3D Vector
//=========================================================
class Vector						// same data-layout as engine's vec3_t,
{								//		which is a vec_t[3]
public:
	// Construction/destruction
	inline Vector(void)								{ }
	inline Vector(float X, float Y, float Z)		{ x = X; y = Y; z = Z;						}
	inline Vector(double X, double Y, double Z)		{ x = (float)X; y = (float)Y; z = (float)Z;	}
	inline Vector(int X, int Y, int Z)				{ x = (float)X; y = (float)Y; z = (float)Z;	}
	inline Vector(const Vector& v)					{ x = v.x; y = v.y; z = v.z;				} 
	inline Vector(float rgfl[3])					{ x = rgfl[0]; y = rgfl[1]; z = rgfl[2];	}

	// Operators
	inline Vector operator-(void) const				{ return Vector(-x,-y,-z);				}
	inline int operator==(const Vector& v) const	{ return x==v.x && y==v.y && z==v.z;	}
	inline int operator!=(const Vector& v) const	{ return !(*this==v);					}
	inline Vector operator+(const Vector& v) const	{ return Vector(x+v.x, y+v.y, z+v.z);	}
	inline Vector operator-(const Vector& v) const	{ return Vector(x-v.x, y-v.y, z-v.z);	}
	inline Vector operator*(float fl) const			{ return Vector(x*fl, y*fl, z*fl);		}
	inline Vector operator/(float fl) const			{ return Vector(x/fl, y/fl, z/fl);		}
	
	// Methods
	inline void CopyToArray(float* rgfl) const		{ rgfl[0] = x, rgfl[1] = y, rgfl[2] = z; }
	inline float Length(void) const					{ return (float)sqrt(x*x + y*y + z*z); }
	operator float *()								{ return &x; } // Vectors will now automatically convert to float * when needed
	operator const float *() const					{ return &x; } // Vectors will now automatically convert to float * when needed
	inline Vector Normalize(void) const {
		float flLen = Length();
		if (flLen == 0) return Vector(0,0,1);
		flLen = 1 / flLen;
		return Vector(x * flLen, y * flLen, z * flLen);
	}
	
	inline Vector2D Make2D ( void ) const {	return Vector2D(x, y); }

	inline float Length2D(void) const { return (float)sqrt(x*x + y*y); }

	inline float Dot(const Vector& v) const { return x * v.x + y * v.y + z * v.z;	}

	inline Vector CrossProduct(const Vector& v) const { return Vector(v.y * z - v.z * y, v.z * x - v.x * z, v.x * y - v.y * x); }

	inline Vector Sum(const Vector& v) const { return Vector(v.x + x, v.y + y, v.z + z); }

	inline void Clear() { x = y = z = 0.f; }

	inline float Distance(const Vector& v) const { return sqrt(POW(v.x - x) + POW(v.y - y) + POW(v.z - z)); }

	// Members
	vec_t x, y, z;
};

#define vec3_t Vector

#endif
