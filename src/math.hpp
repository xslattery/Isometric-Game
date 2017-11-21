#ifndef _MATH_HPP_
#define _MATH_HPP_

#define ZERO_INITIALISE 1

#define SSE_SUPPORT 1
// NOTE(Xavier): (2017.11.21) SSE support needs to be tested, 
// because the 16-byte alignment has not been taken into consideration.
// MAYBE: create a seperate data type like 'sse_vec4'.
#if SSE_SUPPORT
#define SSE_ALIGNED __attribute__((aligned(16)))
#include <x86intrin.h>
#endif

#include <cstddef>	// For - std::size_t
#include <cmath>	// For - sin() & cos() & sqrt()
#include <ostream>	// For - std::ostream

/////////////////////////////////
struct vec2
{
	union
	{
		struct
		{
			float x, y;
		};
		struct
		{
			float elements[2];
		};
	};
	
	/////////////////////////////////
	// NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	// OR should it be uninitialised?
	vec2 ()
	{
	#if ZERO_INITIALISE
		this->x = 0;
		this->y = 0;
	#endif
	}
	
	/////////////////////////////////
	vec2 ( const float& v )
	{
		this->x = v;
		this->y = v;
	}
	
	/////////////////////////////////
	vec2 ( const float& x, const float& y )
	{
		this->x = x;
		this->y = y;
	}

	/////////////////////////////////
	vec2 ( const vec2& v )
	{
		this->x = v.x;
		this->y = v.y;
	}

	/////////////////////////////////
	vec2& operator = ( const vec2& v )
	{
		this->x = v.x;
		this->y = v.y;
		return *this;
	}

	/////////////////////////////////
	vec2& operator += ( const vec2& v )
	{
		this->x += v.x;
		this->y += v.y;
		return *this;
	}

	/////////////////////////////////
	vec2& operator -= ( const vec2& v )
	{
		this->x -= v.x;
		this->y -= v.y;
		return *this;
	}

	/////////////////////////////////
	float& operator [] ( std::size_t idx ) { return this->elements[idx]; }
	
	/////////////////////////////////
	const float operator [] ( std::size_t idx ) const { return this->elements[idx]; }
};

/////////////////////////////////
inline bool operator == ( const vec2& v1, const vec2& v2 )
{
	return ( v1.x == v2.x && v1.y == v2.y );
}

/////////////////////////////////
inline bool operator != ( const vec2& v1, const vec2& v2 )
{
	return !( v1 == v2 );
}

/////////////////////////////////
inline vec2 operator + ( const vec2& v1, const vec2& v2 )
{
	vec2 result { v1.x + v2.x, v1.y + v2.y };
	return result;
}

/////////////////////////////////
inline vec2 operator - ( const vec2& v1, const vec2& v2 )
{
	vec2 result { v1.x - v2.x, v1.y - v2.y };
	return result;
}

/////////////////////////////////
inline vec2 operator - ( const vec2& v1 )
{
	vec2 result { -v1.x, -v1.y };
	return result;
}

/////////////////////////////////
inline vec2 operator * ( const vec2& v1, const vec2& v2 )
{
	vec2 result { v1.x * v2.x, v1.y * v2.y };
	return result;
}

/////////////////////////////////
inline vec2 operator / ( const vec2& v1, const vec2& v2 )
{
	vec2 result { v1.x / v2.x, v1.y / v2.y };
	return result;
}

/////////////////////////////////
inline vec2 operator * ( const vec2& v1, const float& s )
{
	vec2 result { v1.x * s, v1.y * s };
	return result;
}

/////////////////////////////////
inline vec2 operator * ( const float& s, const vec2& v1 )
{
	vec2 result { v1.x * s, v1.y * s };
	return result;
}

/////////////////////////////////
inline vec2 operator / ( const vec2& v1, const float& s )
{
	vec2 result { v1.x / s, v1.y / s };
	return result;
}

/////////////////////////////////
inline vec2 operator / ( const float& s, const vec2& v1 )
{
	vec2 result { v1.x / s, v1.y / s };
	return result;
}

/////////////////////////////////
inline std::ostream& operator << ( std::ostream& os, const vec2& v )
{
	return ( os << "vec2{" << v.x << ", " << v.y << '}' );
}



/////////////////////////////////
struct vec3
{
	union
	{
		struct
		{
			float x, y, z;
		};
		struct
		{
			float elements[3];
		};
		struct
		{
			vec2 xy;
			float _z;
		};
	};
	
	/////////////////////////////////
	// NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	// OR should it be uninitialised?
	vec3 ()
	{
	#if ZERO_INITIALISE
		this->x = 0;
		this->y = 0;
		this->z = 0;
	#endif
	}

	/////////////////////////////////
	vec3 ( const float& v )
	{
		this->x = v;
		this->y = v;
		this->z = v;
	}
	
	/////////////////////////////////
	vec3 ( const float& x, const float& y, const float& z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	/////////////////////////////////
	vec3 ( const vec3& v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	/////////////////////////////////
	vec3 ( const vec2& v, const float& z )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = z;
	}

	/////////////////////////////////
	vec3& operator = ( const vec3& v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}

	/////////////////////////////////
	vec3& operator += ( const vec3& v )
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}

	/////////////////////////////////
	vec3& operator -= ( const vec3& v )
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		return *this;
	}

	/////////////////////////////////
	float& operator [] ( std::size_t idx ) { return this->elements[idx]; }
	
	/////////////////////////////////
	const float operator [] ( std::size_t idx ) const { return this->elements[idx]; }
};

/////////////////////////////////
inline bool operator == ( const vec3& v1, const vec3& v2 )
{
	return ( v1.x == v2.x && v1.y == v2.y && v1.z == v2.z );
}

/////////////////////////////////
inline bool operator != ( const vec3& v1, const vec3& v2 )
{
	return !( v1 == v2 );
}

/////////////////////////////////
inline vec3 operator + ( const vec3& v1, const vec3& v2 )
{
	vec3 result { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	return result;
}

/////////////////////////////////
inline vec3 operator - ( const vec3& v1, const vec3& v2 )
{
	vec3 result { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	return result;
}

/////////////////////////////////
inline vec3 operator - ( const vec3& v1 )
{
	vec3 result { -v1.x, -v1.y, -v1.z };
	return result;
}

/////////////////////////////////
inline vec3 operator * ( const vec3& v1, const vec3& v2 )
{
	vec3 result { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
	return result;
}

/////////////////////////////////
inline vec3 operator / ( const vec3& v1, const vec3& v2 )
{
	vec3 result { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
	return result;
}

/////////////////////////////////
inline vec3 operator * ( const vec3& v1, const float& s )
{
	vec3 result { v1.x * s, v1.y * s, v1.z * s };
	return result;
}

/////////////////////////////////
inline vec3 operator * ( const float& s, const vec3& v1 )
{
	vec3 result { v1.x * s, v1.y * s, v1.z * s };
	return result;
}

/////////////////////////////////
inline vec3 operator / ( const vec3& v1, const float& s )
{
	vec3 result { v1.x / s, v1.y / s, v1.z / s };
	return result;
}

/////////////////////////////////
inline vec3 operator / ( const float& s, const vec3& v1 )
{
	vec3 result { v1.x / s, v1.y / s, v1.z / s };
	return result;
}

/////////////////////////////////
inline std::ostream& operator << ( std::ostream& os, const vec3& v )
{
	return ( os << "vec3{" << v.x << ", " << v.y << ", " << v.z << '}' );
}



/////////////////////////////////
struct vec4
{
	union
	{
		struct
		{
			float x, y, z, w;
		};
		struct
		{
			float elements[4];
		};
		struct
		{
			vec3 xyz;
			float _w;	
		};
		struct
		{
			vec2 xy, zw;
		};
		#if SSE_SUPPORT
			__m128 sse;
		#endif
	};
	
	/////////////////////////////////
	// NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	// OR should it be uninitialised?
	vec4 () 
	{
	#if ZERO_INITIALISE
	#if SSE_SUPPORT
		this->sse = _mm_setzero_ps();
	#else
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	#endif
	#endif
	}

	/////////////////////////////////
	vec4 ( const float& v )
	{
		#if SSE_SUPPORT
			this->sse = _mm_set1_ps( v );
		#else
			this->x = v;
			this->y = v;
			this->z = v;
			this->w = v;
		#endif
	}
	
	/////////////////////////////////
	vec4 ( const float& x, const float& y, const float& z, const float& w )
	{
		#if SSE_SUPPORT
			this->sse = _mm_setr_ps( x, y, z, w );
		#else
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		#endif
	}

	/////////////////////////////////
	vec4 ( const vec4& v )
	{
	#if SSE_SUPPORT
		this->sse = v.sse;
	#else
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
	#endif
	}

	/////////////////////////////////
	vec4 ( const vec3& v, const float& w )
	{
		this->xyz = v;
		this->w = w;
	}

	/////////////////////////////////
	vec4 ( const vec2& v1, const vec2& v2 )
	{
		this->xy = v1;
		this->zw = v2;
	}

	/////////////////////////////////
	vec4 ( const vec2& v, const float& z, const float& w )
	{
		this->xy = v;
		this->z = z;
		this->w = w;
	}

	/////////////////////////////////
	vec4& operator = ( const vec4& v )
	{
	#if SSE_SUPPORT
		this->sse = v.sse;
	#else
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
	#endif
		return *this;
	}

	/////////////////////////////////
	vec4& operator += ( const vec4& v )
	{
	#if SSE_SUPPORT
		this->sse = _mm_add_ps( this->sse, v.sse );
	#else
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		this->w += v.w;
	#endif
		return *this;
	}

	/////////////////////////////////
	vec4& operator -= ( const vec4& v )
	{
	#if SSE_SUPPORT
		this->sse = _mm_sub_ps( this->sse, v.sse );
	#else
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		this->w -= v.w;
	#endif
		return *this;
	}

	/////////////////////////////////
	float& operator [] ( std::size_t idx ) { return this->elements[idx]; }
	
	/////////////////////////////////
	const float operator [] ( std::size_t idx ) const { return this->elements[idx]; }
};

/////////////////////////////////
inline bool operator == ( const vec4& v1, const vec4& v2 )
{
	return ( v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w );
}

/////////////////////////////////
inline bool operator != ( const vec4& v1, const vec4& v2 )
{
	return !( v1 == v2 );
}

/////////////////////////////////
inline vec4 operator + ( const vec4& v1, const vec4& v2 )
{
#if SSE_SUPPORT
	vec4 result;
	result.sse = _mm_add_ps( v1.sse, v2.sse );
#else
	vec4 result = v1;
	result.x += v2.x;
	result.y += v2.y;
	result.z += v2.z;
	result.w += v2.w;
#endif
	return result;
}

/////////////////////////////////
inline vec4 operator - ( const vec4& v1, const vec4& v2 )
{
#if SSE_SUPPORT
	vec4 result;
	result.sse = _mm_sub_ps( v1.sse, v2.sse );
#else
	vec4 result = v1;
	result.x -= v2.x;
	result.y -= v2.y;
	result.z -= v2.z;
	result.w -= v2.w;
#endif
	return result;
}

/////////////////////////////////
inline vec4 operator - ( const vec4& v1 )
{
#if SSE_SUPPORT
	vec4 result;
	__m128 zero = _mm_set1_ps( 0 );
	result.sse = _mm_sub_ps( zero, v1.sse);
#else
	vec4 result;
	result.x = -v1.x;
	result.y = -v1.y;
	result.z = -v1.z;
	result.w = -v1.w;
#endif
	return result;
}

/////////////////////////////////
inline vec4 operator * ( const vec4& v1, const vec4& v2 )
{
#if SSE_SUPPORT
	vec4 result;
	result.sse = _mm_mul_ps( v1.sse, v2.sse );
#else
	vec4 result = v1;
	result.x *= v2.x;
	result.y *= v2.y;
	result.z *= v2.z;
	result.w *= v2.w;
#endif
	return result;;
}

/////////////////////////////////
inline vec4 operator / ( const vec4& v1, const vec4& v2 )
{
#if SSE_SUPPORT
	vec4 result;
	result.sse = _mm_div_ps( v1.sse, v2.sse );
#else
	vec4 result = v1;
	result.x /= v2.x;
	result.y /= v2.y;
	result.z /= v2.z;
	result.w /= v2.w;
#endif
	return result;;
}

/////////////////////////////////
inline vec4 operator * ( const vec4& v1, const float& s )
{
#if SSE_SUPPORT
	vec4 result;
	__m128 scalar = _mm_set1_ps( s );
	result.sse = _mm_mul_ps( v1.sse, scalar);
#else
	vec4 result = v1;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	result.w *= s;
#endif
	return result;
}

/////////////////////////////////
inline vec4 operator * ( const float& s, const vec4& v1 )
{
#if SSE_SUPPORT
	vec4 result;
	__m128 scalar = _mm_set1_ps( s );
	result.sse = _mm_mul_ps( v1.sse, scalar);
#else
	vec4 result = v1;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	result.w *= s;
#endif
	return result;
}

/////////////////////////////////
inline vec4 operator / ( const vec4& v1, const float& s )
{
#if SSE_SUPPORT
	vec4 result;
	__m128 scalar = _mm_set1_ps( s );
	result.sse = _mm_div_ps( v1.sse, scalar);
#else
	vec4 result = v1;
	result.x /= s;
	result.y /= s;
	result.z /= s;
	result.w /= s;
#endif
	return result;
}

/////////////////////////////////
inline vec4 operator / ( const float& s, const vec4& v1 )
{
#if SSE_SUPPORT
	vec4 result;
	__m128 scalar = _mm_set1_ps( s );
	result.sse = _mm_div_ps( v1.sse, scalar);
#else
	vec4 result = v1;
	result.x /= s;
	result.y /= s;
	result.z /= s;
	result.w /= s;
#endif
	return result;
}

/////////////////////////////////
inline std::ostream& operator << ( std::ostream& os, const vec4& v )
{
	return ( os << "vec4{" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << '}' );
}



/////////////////////////////////
struct mat4
{
	union
	{
		struct
		{
			vec4 _vec[4];
		};
		struct 
		{
			float elements[16];
		};
	};

	mat4 () // NOTE(Xavier): (2017.11.19) Should this be the identity matrix or not?
	{
		_vec[0] = { 1,0,0,0 };
		_vec[1] = { 0,1,0,0 };
		_vec[2] = { 0,0,1,0 };
		_vec[3] = { 0,0,0,1 };
	}

	mat4 ( float v ) 
	{
		_vec[0] = { v,0,0,0 };
		_vec[1] = { 0,v,0,0 };
		_vec[2] = { 0,0,v,0 };
		_vec[3] = { 0,0,0,v };
	}

	mat4 ( const mat4& m )
	{
		_vec[0] = m._vec[0];
		_vec[1] = m._vec[1];
		_vec[2] = m._vec[2];
		_vec[3] = m._vec[3];
	}

	/////////////////////////////////
	mat4& operator = ( const mat4& m )
	{
		this->_vec[0] = m._vec[0];
		this->_vec[1] = m._vec[1];
		this->_vec[2] = m._vec[2];
		this->_vec[3] = m._vec[3];
		return *this;
	}

	/////////////////////////////////
	vec4& operator [] ( std::size_t idx ) { return this->_vec[idx]; }
	
	/////////////////////////////////
	const vec4 operator [] ( std::size_t idx ) const { return this->_vec[idx]; }
};

/////////////////////////////////
inline bool operator == ( const mat4& m1, const mat4& m2 )
{
	return ( m1._vec[0] == m2._vec[0] && m1._vec[1] == m2._vec[1] && m1._vec[2] == m2._vec[2] && m1._vec[3] == m2._vec[3] );
}

/////////////////////////////////
inline bool operator != ( const mat4& m1, const mat4& m2 )
{
	return !( m1 == m2 );
}

/////////////////////////////////
inline mat4 orthographic_projection ( const float b, const float t, const float l, const float r, const float n, const float f ) 
{
	mat4 m;

	// m[0][0] = 2.0f / (r - l);
	// m[0][1] = 0;
	// m[0][2] = 0;
	// m[0][3] = 0;

	// m[1][0] = 0;
	// m[1][1] = 2.0f / (t - b);
	// m[1][2] = 0;
	// m[1][3] = 0;

	// m[2][0] = 0;
	// m[2][1] = 0;
	// m[2][2] = -2.0f / (f - n);
	// m[2][3] = 0;

	// m[3][0] = -(r + l) / (r - l);
	// m[3][1] = -(t + b) / (t - b);
	// m[3][2] = -(f + n) / (f - n);
	// m[3][3] = 1.0f;

	m[0][0] = 2 / (r - l);
	m[1][1] = 2 / (t - b);
	m[2][2] = -2 / (f - n);
	m[3][0] = -(r + l) / (r - l);
	m[3][1] = -(t + b) / (t - b);
	m[3][2] = -(f + n) / (f - n);

	return m;
}



/////////////////////////////////
inline vec2 normalize ( const vec2& v )
{
	const float s = sqrt( v.x*v.x + v.y*v.y );
	return v * s;
}

/////////////////////////////////
inline vec3 normalize ( const vec3& v )
{
	const float s = sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
	return v * s;
}

/////////////////////////////////
inline vec4 normalize ( const vec4& v )
{
	const float s = sqrt( v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w );
	return v * s;
}

//////////////////////
// Vector Functions:
// TODO(Xavier): distance() - the distance between two points.
// TODO(Xavier): length() - the length of a vector.
// TODO(Xavier): rotate() - rotate a vector.
// TODO(Xavier): dot_product()
// TODO(Xavier): cross_product()

/////////////////////////////////
inline mat4 translate ( const mat4& m, const vec3& dir )
{
	mat4 result = m;

	result[3].xyz += dir;

	return result;
}

/////////////////////////////////
inline mat4 scale ( const mat4& m, const vec3& s )
{
	mat4 result = m;

	result[0][0] *= s[0];
	result[1][1] *= s[1];
	result[2][2] *= s[2];

	return result;
}

inline mat4 rotate ( const mat4& m, const vec3& dir, const float& a )
{
	const float s = sin( a );
	const float c = cos( a );

	const vec3 axis( normalize(dir) );
	const vec3 temp( (vec3(1) - c) * axis );

	mat4 rotate(0);
	rotate[0][0] = c + temp[0] * axis[0];
	rotate[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
	rotate[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

	rotate[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
	rotate[1][1] = c + temp[1] * axis[1];
	rotate[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

	rotate[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
	rotate[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
	rotate[2][2] = c + temp[2] * axis[2];

	mat4 result{0};
	result[0] = m[0] * rotate[0][0] + m[1] * rotate[0][1] + m[2] * rotate[0][2];
	result[1] = m[0] * rotate[1][0] + m[1] * rotate[1][1] + m[2] * rotate[1][2];
	result[2] = m[0] * rotate[2][0] + m[1] * rotate[2][1] + m[2] * rotate[2][2];
	result[3] = m[3];
	
	return result;
}

#endif