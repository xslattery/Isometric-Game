#ifndef _MATH_HPP_
#define _MATH_HPP_

#include <cstddef>
#include <cmath>
#include <ostream>

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
	vec2 () // NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	{
		this->x = 0;
		this->y = 0;
	}
	
	/////////////////////////////////
	vec2 ( float v )
	{
		this->x = v;
		this->y = v;
	}
	
	/////////////////////////////////
	vec2 ( float x, float y )
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
inline vec2& operator + ( vec2& v1, const vec2& v2 )
{
	v1.x += v2.x;
	v1.y += v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator + ( vec2&& v1, const vec2& v2 )
{
	v1.x += v2.x;
	v1.y += v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator - ( vec2& v1, const vec2& v2 )
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator - ( vec2&& v1, const vec2& v2 )
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator * ( vec2& v1, const vec2& v2 )
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator * ( vec2&& v1, const vec2& v2 )
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator / ( vec2& v1, const vec2& v2 )
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator / ( vec2&& v1, const vec2& v2 )
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	return v1;
}

/////////////////////////////////
inline vec2& operator * ( vec2& v1, const float s )
{
	v1.x *= s;
	v1.y *= s;
	return v1;
}

/////////////////////////////////
inline vec2& operator * ( const float s, vec2& v1 )
{
	v1.x *= s;
	v1.y *= s;
	return v1;
}

/////////////////////////////////
inline vec2& operator * ( vec2&& v1, const float s )
{
	v1.x *= s;
	v1.y *= s;
	return v1;
}

/////////////////////////////////
inline vec2& operator * ( const float s, vec2&& v1 )
{
	v1.x *= s;
	v1.y *= s;
	return v1;
}

/////////////////////////////////
inline vec2 operator * ( const vec2& v1, const float s )
{
	vec2 v = v1;
	v.x *= s;
	v.y *= s;
	return v;
}

/////////////////////////////////
inline vec2 operator * ( const float s, const vec2& v1 )
{
	vec2 v = v1;
	v.x *= s;
	v.y *= s;
	return v;
}

/////////////////////////////////
inline vec2& operator / ( vec2& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	return v1;
}

/////////////////////////////////
inline vec2& operator / ( vec2&& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	return v1;
}

/////////////////////////////////
inline vec2 operator / ( const vec2& v1, const float s )
{
	vec2 v = v1;
	v.x /= s;
	v.y /= s;
	return v;
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
	vec3 () // NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	/////////////////////////////////
	vec3 ( float v )
	{
		this->x = v;
		this->y = v;
		this->z = v;
	}
	
	/////////////////////////////////
	vec3 ( float x, float y, float z )
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
	vec3 ( const vec2& v, float z )
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
inline vec3& operator + ( vec3& v1, const vec3& v2 )
{
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator + ( vec3&& v1, const vec3& v2 )
{
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator - ( vec3& v1, const vec3& v2 )
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator - ( vec3&& v1, const vec3& v2 )
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator * ( vec3& v1, const vec3& v2 )
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator * ( vec3&& v1, const vec3& v2 )
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator / ( vec3& v1, const vec3& v2 )
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator / ( vec3&& v1, const vec3& v2 )
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
	return v1;
}

/////////////////////////////////
inline vec3& operator * ( vec3& v1, const float s )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	return v1;
}

/////////////////////////////////
inline vec3& operator * ( const float s, vec3& v1 )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	return v1;
}

/////////////////////////////////
inline vec3& operator * ( vec3&& v1, const float s )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	return v1;
}

/////////////////////////////////
inline vec3& operator * ( const float s, vec3&& v1 )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	return v1;
}

/////////////////////////////////
inline vec3 operator * ( const vec3& v1, const float s )
{
	vec3 v = v1;
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

/////////////////////////////////
inline vec3 operator * ( const float s, const vec3& v1 )
{
	vec3 v = v1;
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

/////////////////////////////////
inline vec3& operator / ( vec3& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	v1.z /= s;
	return v1;
}

/////////////////////////////////
inline vec3& operator / ( vec3&& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	v1.z /= s;
	return v1;
}

/////////////////////////////////
inline vec3 operator / ( const vec3& v1, const float s )
{
	vec3 v = v1;
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
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
	};
	
	/////////////////////////////////
	vec4 () // NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}

	/////////////////////////////////
	vec4 ( float v )
	{
		this->x = v;
		this->y = v;
		this->z = v;
		this->w = v;
	}
	
	/////////////////////////////////
	vec4 ( float x, float y, float z, float w )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	/////////////////////////////////
	vec4 ( const vec4& v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
	}

	/////////////////////////////////
	vec4 ( const vec3& v, float w )
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
	vec4 ( const vec2& v, float z, float w )
	{
		this->xy = v;
		this->z = z;
		this->w = w;
	}

	/////////////////////////////////
	vec4& operator = ( const vec4& v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
		return *this;
	}

	/////////////////////////////////
	vec4& operator += ( const vec4& v )
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		this->w += v.w;
		return *this;
	}

	/////////////////////////////////
	vec4& operator -= ( const vec4& v )
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		this->w -= v.w;
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
inline vec4& operator + ( vec4& v1, const vec4& v2 )
{
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	v1.w += v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator + ( vec4&& v1, const vec4& v2 )
{
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	v1.w += v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator - ( vec4& v1, const vec4& v2 )
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	v1.w -= v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator - ( vec4&& v1, const vec4& v2 )
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	v1.w -= v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator * ( vec4& v1, const vec4& v2 )
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;
	v1.w *= v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator * ( vec4&& v1, const vec4& v2 )
{
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;
	v1.w *= v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator / ( vec4& v1, const vec4& v2 )
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
	v1.w /= v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator / ( vec4&& v1, const vec4& v2 )
{
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
	v1.w /= v2.w;
	return v1;
}

/////////////////////////////////
inline vec4& operator * ( vec4& v1, const float s )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	v1.w *= s;
	return v1;
}

/////////////////////////////////
inline vec4& operator * ( const float s, vec4& v1 )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	v1.w *= s;
	return v1;
}

/////////////////////////////////
inline vec4& operator * ( vec4&& v1, const float s )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	v1.w *= s;
	return v1;
}

/////////////////////////////////
inline vec4& operator * ( const float s, vec4&& v1 )
{
	v1.x *= s;
	v1.y *= s;
	v1.z *= s;
	v1.w *= s;
	return v1;
}

/////////////////////////////////
inline vec4 operator * ( const vec4& v1, const float s )
{
	vec4 v = v1;
	v.x *= s;
	v.y *= s;
	v.z *= s;
	v.w *= s;
	return v;
}

/////////////////////////////////
inline vec4 operator * ( const float s, const vec4& v1 )
{
	vec4 v = v1;
	v.x *= s;
	v.y *= s;
	v.z *= s;
	v.w *= s;
	return v;
}

/////////////////////////////////
inline vec4& operator / ( vec4& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	v1.z /= s;
	v1.w /= s;
	return v1;
}

/////////////////////////////////
inline vec4& operator / ( vec4&& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	v1.z /= s;
	v1.w /= s;
	return v1;
}

/////////////////////////////////
inline vec4 operator / ( const vec4& v1, const float s )
{
	vec4 v = v1;
	v.x /= s;
	v.y /= s;
	v.z /= s;
	v.w /= s;
	return v;
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

// NOTE(Xavier): (2017.11.19) The following functions should probably be moved
// into their own .cpp file. ALSO, the 'normalize' functions should probably
// have support added for r-value and non-const reference arguments with reference return values.

/////////////////////////////////
inline vec2 normalize ( const vec2& v )
{
	float s = sqrt( v.x*v.x + v.y*v.y );
	return vec2( v ) * s;
}

/////////////////////////////////
inline vec3 normalize ( const vec3& v )
{
	float s = sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
	return vec3( v ) * s;
}

/////////////////////////////////
inline vec4 normalize ( const vec4& v )
{
	float s = sqrt( v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w );
	return vec4( v ) * s;
}

/////////////////////////////////
inline mat4 translate ( const mat4& m, const vec3& dir )
{
	mat4 mat = m;

	mat[3].xyz += dir;

	return mat;
}

/////////////////////////////////
inline mat4 scale ( const mat4& m, const vec3& s )
{
	mat4 mat = m;

	mat[0][0] *= s[0];
	mat[1][1] *= s[1];
	mat[2][2] *= s[2];

	return mat;
}

inline mat4 rotate ( const mat4& m, const vec3& v, float a )
{
	const float s = sin( a );
	const float c = cos( a );

	const vec3 axis( normalize(v) );
	const vec3 temp( (vec3(1) - c) * axis );

	mat4 Rotate(0);
	Rotate[0][0] = c + temp[0] * axis[0];
	Rotate[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
	Rotate[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

	Rotate[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
	Rotate[1][1] = c + temp[1] * axis[1];
	Rotate[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

	Rotate[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
	Rotate[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
	Rotate[2][2] = c + temp[2] * axis[2];

	mat4 Result(0);
	Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
	Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
	Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
	Result[3] = m[3];
	
	return Result;
}

#endif