#ifndef _MATH_HPP_
#define _MATH_HPP_

#include <cstddef>
#include <ostream>

/////////////////////////////////
struct vec2
{
	union
	{
		float x, y;
		struct
		{
			float elements[2];
		};
	};
	
	/////////////////////////////////
	vec2() { } // NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	
	/////////////////////////////////
	vec2( float v )
	{
		this->x = v;
		this->y = v;
	}
	
	/////////////////////////////////
	vec2( float x, float y )
	{
		this->x = x;
		this->y = y;
	}

	/////////////////////////////////
	vec2( const vec2& v )
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
inline vec2& operator / ( vec2& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	return v1;
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
		float x, y, z;
		struct
		{
			float elements[3];
		};
	};
	
	/////////////////////////////////
	vec3() { } // NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	
	/////////////////////////////////
	vec3( float v )
	{
		this->x = v;
		this->y = v;
		this->z = v;
	}
	
	/////////////////////////////////
	vec3( float x, float y, float z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	/////////////////////////////////
	vec3( const vec3& v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
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
inline vec3& operator / ( vec3& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	v1.z /= s;
	return v1;
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
		float x, y, z, w;
		struct
		{
			float elements[4];
		};
	};
	
	/////////////////////////////////
	vec4() { } // NOTE(Xavier): (2017.11.16) Should this be 0 initialised?
	
	/////////////////////////////////
	vec4( float v )
	{
		this->x = v;
		this->y = v;
		this->z = v;
		this->w = w;
	}
	
	/////////////////////////////////
	vec4( float x, float y, float z, float w )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w += w;
	}

	/////////////////////////////////
	vec4( const vec4& v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w += v.w;
	}

	/////////////////////////////////
	vec4& operator = ( const vec4& v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w += v.w;
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
inline vec4& operator / ( vec4& v1, const float s )
{
	v1.x /= s;
	v1.y /= s;
	v1.z /= s;
	v1.w /= s;
	return v1;
}

/////////////////////////////////
inline std::ostream& operator << ( std::ostream& os, const vec4& v )
{
	return ( os << "vec4{" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << '}' );
}

#endif