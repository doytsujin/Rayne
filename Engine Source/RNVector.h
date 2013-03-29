//
//  RNVector.h
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_VECTOR_H__
#define __RAYNE_VECTOR_H__

#include "RNBase.h"

namespace RN
{
	class Vector3;
	class Vector4;
	
	class Vector2
	{
	public:
		RNAPI Vector2();
		RNAPI Vector2(const float n);
		RNAPI Vector2(const float x, const float y);
		RNAPI Vector2(const Vector3& other);
		RNAPI Vector2(const Vector4& other);

		RNAPI bool operator== (const Vector2 &other) const;
		RNAPI bool operator!= (const Vector2 &other) const;

		RNAPI Vector2 operator- () const;

		RNAPI Vector2 operator+ (const Vector2& other) const;
		RNAPI Vector2 operator- (const Vector2& other) const;
		RNAPI Vector2 operator* (const Vector2& other) const;
		RNAPI Vector2 operator/ (const Vector2& other) const;
		RNAPI Vector2 operator* (const float n) const;
		RNAPI Vector2 operator/ (const float n) const;

		RNAPI Vector2& operator+= (const Vector2& other);
		RNAPI Vector2& operator-= (const Vector2& other);
		RNAPI Vector2& operator*= (const Vector2& other);
		RNAPI Vector2& operator/= (const Vector2& other);

		RNAPI float Length() const;
		RNAPI float Dot (const Vector2& other) const;
		RNAPI bool IsEqual(const Vector2& other, float epsilon) const;

		RNAPI Vector2& Normalize(const float n=1.0f);

		struct
		{
			float x;
			float y;
		};
	};

	class Vector3
	{
	public:
		RNAPI Vector3();
		RNAPI Vector3(const float n);
		RNAPI Vector3(const float x, const float y, const float z);
		RNAPI Vector3(const Vector2& other, float z=0.0f);
		RNAPI Vector3(const Vector4& other);
		
		RNAPI bool operator== (const Vector3 &other) const;
		RNAPI bool operator!= (const Vector3 &other) const;
		
		RNAPI Vector3 operator- () const;
		
		RNAPI Vector3 operator+ (const Vector3& other) const;
		RNAPI Vector3 operator- (const Vector3& other) const;
		RNAPI Vector3 operator* (const Vector3& other) const;
		RNAPI Vector3 operator/ (const Vector3& other) const;
		RNAPI Vector3 operator* (const float n) const;
		RNAPI Vector3 operator/ (const float n) const;
		
		RNAPI Vector3& operator+= (const Vector3& other);
		RNAPI Vector3& operator-= (const Vector3& other);
		RNAPI Vector3& operator*= (const Vector3& other);
		RNAPI Vector3& operator/= (const Vector3& other);
		
		RNAPI float Length() const;
		RNAPI float Dot(const Vector3& other) const;
		RNAPI Vector3 Cross(const Vector3& other) const;
		RNAPI bool IsEqual(const Vector3& other, float epsilon) const;
		RNAPI float Distance(const Vector3 &other) const;
		RNAPI Vector3 Lerp(const Vector3 &other, float factor) const;
		
		RNAPI Vector3& Normalize(const float n=1.0f);
		
		struct
		{
			float x;
			float y;
			float z;
		};
	};
	
	class Vector4
	{
	public:
		RNAPI Vector4();
		RNAPI Vector4(const float n);
		RNAPI Vector4(const float x, const float y, const float z, const float w);
		RNAPI Vector4(const Vector2& other, float z=0.0f, float w=0.0f);
		RNAPI Vector4(const Vector3& other, float w=0.0f);
		
		RNAPI bool operator== (const Vector4 &other) const;
		RNAPI bool operator!= (const Vector4 &other) const;
		
		RNAPI Vector4 operator- () const;
		
		RNAPI Vector4 operator+ (const Vector4& other) const;
		RNAPI Vector4 operator- (const Vector4& other) const;
		RNAPI Vector4 operator* (const Vector4& other) const;
		RNAPI Vector4 operator/ (const Vector4& other) const;
		RNAPI Vector4 operator* (const float n) const;
		RNAPI Vector4 operator/ (const float n) const;
		
		RNAPI Vector4& operator+= (const Vector4& other);
		RNAPI Vector4& operator-= (const Vector4& other);
		RNAPI Vector4& operator*= (const Vector4& other);
		RNAPI Vector4& operator/= (const Vector4& other);
		
		RNAPI float Length() const;
		RNAPI float Dot(const Vector4& other) const;
		RNAPI Vector4 Cross(const Vector4& other) const;
		RNAPI bool IsEqual(const Vector4& other, float epsilon) const;
		
		RNAPI Vector4& Normalize(const float n=1.0f);
		
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
	};
	
	
	
	RN_INLINE Vector2::Vector2()
	{
		x = y = 0.0f;
	}

	RN_INLINE Vector2::Vector2(const float n)
	{
		x = y = n;
	}

	RN_INLINE Vector2::Vector2(const float _x, const float _y)
	{
		x = _x;
		y = _y;
	}
	
	RN_INLINE Vector2::Vector2(const Vector3& other)
	{
		x = other.x;
		y = other.y;
	}
	
	RN_INLINE Vector2::Vector2(const Vector4& other)
	{
		x = other.x;
		y = other.y;
	}

	RN_INLINE bool Vector2::operator== (const Vector2 &other) const
	{
		if(fabs(x - other.x) > kRNEpsilonFloat)
			return false;

		if(fabs(y - other.y) > kRNEpsilonFloat)
			return false;

		return true;
	}

	RN_INLINE bool Vector2::operator!= (const Vector2 &other) const
	{
		if(fabs(x - other.x) <= kRNEpsilonFloat && fabs(y - other.y) <= kRNEpsilonFloat)
			return false;

		return true;
	}

	RN_INLINE Vector2 Vector2::operator- () const
	{
		return Vector2(-x, -y);
	}

	RN_INLINE Vector2 Vector2::operator+ (const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}
	RN_INLINE Vector2 Vector2::operator- (const Vector2& other) const
	{
		return Vector2(x - other.x, y - other.y);
	}
	RN_INLINE Vector2 Vector2::operator* (const Vector2& other) const
	{
		return Vector2(x * other.x, y * other.y);
	}
	RN_INLINE Vector2 Vector2::operator/ (const Vector2& other) const
	{
		return Vector2(x / other.x, y / other.y);
	}
	RN_INLINE Vector2 Vector2::operator* (const float n) const
	{
		return Vector2(x * n, y * n);
	}
	RN_INLINE Vector2 Vector2::operator/ (const float n) const
	{
		return Vector2(x / n, y / n);
	}

	RN_INLINE Vector2& Vector2::operator+= (const Vector2& other)
	{
		x += other.x;
		y += other.y;

		return *this;
	}
	RN_INLINE Vector2& Vector2::operator-= (const Vector2& other)
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}
	RN_INLINE Vector2& Vector2::operator*= (const Vector2& other)
	{
		x *= other.x;
		y *= other.y;

		return *this;
	}
	RN_INLINE Vector2& Vector2::operator/= (const Vector2& other)
	{
		x /= other.x;
		y /= other.y;

		return *this;
	}

	RN_INLINE float Vector2::Length() const
	{
		return sqrtf(x * x + y * y);
	}

	RN_INLINE float Vector2::Dot(const Vector2& other) const
	{
		return (x * other.x + y * other.y);
	}

	RN_INLINE bool Vector2::IsEqual(const Vector2& other, float epsilon) const
	{
		if(fabs(x - other.x) > epsilon)
			return false;

		if(fabs(y - other.y) > epsilon)
			return false;

		return true;
	}

	RN_INLINE Vector2& Vector2::Normalize(const float n)
	{
		float length = Length();
		x *= n/length;
		y *= n/length;

		return *this;
	}

	
	

	RN_INLINE Vector3::Vector3()
	{
		x = y = z = 0.0f;
	}

	RN_INLINE Vector3::Vector3(const float n)
	{
		x = y = z = n;
	}

	RN_INLINE Vector3::Vector3(const float _x, const float _y, const float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	
	RN_INLINE Vector3::Vector3(const Vector2& other, float _z)
	{
		x = other.x;
		y = other.y;
		z = _z;
	}
	
	RN_INLINE Vector3::Vector3(const Vector4& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}

	RN_INLINE bool Vector3::operator== (const Vector3 &other) const
	{
		if(fabs(x - other.x) > kRNEpsilonFloat)
			return false;

		if(fabs(y - other.y) > kRNEpsilonFloat)
			return false;

		if(fabs(z - other.z) > kRNEpsilonFloat)
			return false;

		return true;
	}

	RN_INLINE bool Vector3::operator!= (const Vector3 &other) const
	{
		if(fabs(x - other.x) <= kRNEpsilonFloat && fabs(y - other.y) <= kRNEpsilonFloat && fabs(z - other.z) <= kRNEpsilonFloat)
			return false;

		return true;
	}

	RN_INLINE Vector3 Vector3::operator- () const
	{
		return Vector3(-x, -y, -z);
	}

	RN_INLINE Vector3 Vector3::operator+ (const Vector3& other) const
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	RN_INLINE Vector3 Vector3::operator- (const Vector3& other) const
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	RN_INLINE Vector3 Vector3::operator* (const Vector3& other) const
	{
		return Vector3(x * other.x, y * other.y, z * other.z);
	}
	RN_INLINE Vector3 Vector3::operator/ (const Vector3& other) const
	{
		return Vector3(x / other.x, y / other.y, z / other.z);
	}
	RN_INLINE Vector3 Vector3::operator* (const float n) const
	{
		return Vector3(x * n, y * n, z * n);
	}
	RN_INLINE Vector3 Vector3::operator/ (const float n) const
	{
		return Vector3(x / n, y / n, z / n);
	}

	RN_INLINE Vector3& Vector3::operator+= (const Vector3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}
	RN_INLINE Vector3& Vector3::operator-= (const Vector3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}
	RN_INLINE Vector3& Vector3::operator*= (const Vector3& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;

		return *this;
	}
	RN_INLINE Vector3& Vector3::operator/= (const Vector3& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;

		return *this;
	}

	RN_INLINE float Vector3::Length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	RN_INLINE float Vector3::Dot(const Vector3& other) const
	{
		return (x * other.x + y * other.y + z * other.z);
	}

	RN_INLINE Vector3 Vector3::Cross(const Vector3& other) const
	{
		Vector3 result;

		result.x = y * other.z - z * other.y;
		result.y = z * other.x - x * other.z;
		result.z = x * other.y - y * other.x;

		return result;
	}

	RN_INLINE bool Vector3::IsEqual(const Vector3& other, float epsilon) const
	{
		if(fabs(x - other.x) > epsilon)
			return false;

		if(fabs(y - other.y) > epsilon)
			return false;

		if(fabs(z - other.z) > epsilon)
			return false;

		return true;
	}

	RN_INLINE Vector3& Vector3::Normalize(const float n)
	{
		float length = n/Length();
		x *= length;
		y *= length;
		z *= length;

		return *this;
	}

	RN_INLINE float Vector3::Distance(const Vector3 &other) const
	{
		Vector3 difference = *this-other;
		return difference.Length();
	}

	RN_INLINE Vector3 Vector3::Lerp(const Vector3 &other, float factor) const
	{
		return *this*(1.0f-factor)+other*factor;
	}




	RN_INLINE Vector4::Vector4()
	{
		x = y = z = w = 0.0f;
	}

	RN_INLINE Vector4::Vector4(const float n)
	{
		x = y = z = w = n;
	}

	RN_INLINE Vector4::Vector4(const float _x, const float _y, const float _z, const float _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	
	RN_INLINE Vector4::Vector4(const Vector2& other, float _z, float _w)
	{
		x = other.x;
		y = other.y;
		z = _z;
		w = _w;
	}
	
	RN_INLINE Vector4::Vector4(const Vector3& other, float _w)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = _w;
	}

	RN_INLINE bool Vector4::operator== (const Vector4 &other) const
	{
		if(fabs(x - other.x) > kRNEpsilonFloat)
			return false;

		if(fabs(y - other.y) > kRNEpsilonFloat)
			return false;

		if(fabs(z - other.z) > kRNEpsilonFloat)
			return false;

		if(fabs(w - other.w) > kRNEpsilonFloat)
			return false;

		return true;
	}

	RN_INLINE bool Vector4::operator!= (const Vector4 &other) const
	{
		if(fabs(x - other.x) <= kRNEpsilonFloat && fabs(y - other.y) <= kRNEpsilonFloat && fabs(z - other.z) <= kRNEpsilonFloat && fabs(w - other.w) <= kRNEpsilonFloat)
			return false;

		return true;
	}

	RN_INLINE Vector4 Vector4::operator- () const
	{
		return Vector4(-x, -y, -z, -w);
	}

	RN_INLINE Vector4 Vector4::operator+ (const Vector4& other) const
	{
		return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
	}
	RN_INLINE Vector4 Vector4::operator- (const Vector4& other) const
	{
		return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
	}
	RN_INLINE Vector4 Vector4::operator* (const Vector4& other) const
	{
		return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
	}
	RN_INLINE Vector4 Vector4::operator/ (const Vector4& other) const
	{
		return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
	}
	RN_INLINE Vector4 Vector4::operator* (const float n) const
	{
		return Vector4(x * n, y * n, z * n, w * n);
	}
	RN_INLINE Vector4 Vector4::operator/ (const float n) const
	{
		return Vector4(x / n, y / n, z / n, w / n);
	}

	RN_INLINE Vector4& Vector4::operator+= (const Vector4& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;

		return *this;
	}

	RN_INLINE Vector4& Vector4::operator-= (const Vector4& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;

		return *this;
	}

	RN_INLINE Vector4& Vector4::operator*= (const Vector4& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;

		return *this;
	}

	RN_INLINE Vector4& Vector4::operator/= (const Vector4& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;

		return *this;
	}

	RN_INLINE float Vector4::Length() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	RN_INLINE float Vector4::Dot (const Vector4& other) const
	{
		return (x * other.x + y * other.y + z * other.z + w * other.w);
	}

	RN_INLINE Vector4 Vector4::Cross(const Vector4& other) const
	{
		Vector4 result;

		result.x = y * other.z - z * other.y;
		result.y = z * other.x - x * other.z;
		result.z = x * other.y - y * other.x;
		result.w = 0.0f;

		return result;
	}

	RN_INLINE bool Vector4::IsEqual(const Vector4& other, float epsilon) const
	{
		if(fabs(x - other.x) > epsilon)
			return false;

		if(fabs(y - other.y) > epsilon)
			return false;

		if(fabs(z - other.z) > epsilon)
			return false;

		if(fabs(w - other.w) > epsilon)
			return false;

		return true;
	}

	RN_INLINE Vector4& Vector4::Normalize (const float n)
	{
		float length = n/Length();
		x *= length;
		y *= length;
		z *= length;
		w *= length;

		return *this;
	}

#ifndef __GNUG__
	static_assert(std::is_trivially_copyable<Vector2>::value, "Vector2 must be trivially copyable");
	static_assert(std::is_trivially_copyable<Vector3>::value, "Vector3 must be trivially copyable");
	static_assert(std::is_trivially_copyable<Vector4>::value, "Vector4 must be trivially copyable");
#endif
}

#endif /* __RAYNE_VECTOR_H__ */
