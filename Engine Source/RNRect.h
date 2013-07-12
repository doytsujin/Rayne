//
//  RNRect.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_RECT_H__
#define __RAYNE_RECT_H__

#include "RNBase.h"
#include "RNVector.h"

namespace RN
{
	class Rect
	{
	public:
		RNAPI Rect();
		RNAPI Rect(float x, float y, float width, float height);
		RNAPI Rect(const Vector2& origin, float width, float height);
		RNAPI Rect(const Vector2& origin, const Vector2& size);
		RNAPI Rect(const Rect& other);
		
		RNAPI bool operator== (const Rect& other);
		RNAPI bool operator!= (const Rect& other);
		
		RNAPI bool ContainsPoint(const Vector2& point) const;
		RNAPI bool IntersectsRect(const Rect& other) const;
		RNAPI bool ContainsRect(const Rect& other) const;
		
		RNAPI Rect& Inset(float dx, float dy);
		RNAPI Rect& Integral();
		
		RNAPI float Top() const;
		RNAPI float Bottom() const;
		RNAPI float Left() const;
		RNAPI float Right() const;
		
		RNAPI Vector2 Origin() const
		{
			return Vector2(x, y);
		}
		
		RNAPI Vector2 Size() const
		{
			return Vector2(width, height);
		}
		
		struct
		{
			float x;
			float y;
			
			float width;
			float height;
		};
	};
	
	RN_INLINE Rect::Rect()
	{
		x = y = width = height = 0.0f;
	}
	
	RN_INLINE Rect::Rect(float tx, float ty, float twidth, float theight)
	{
		x = tx;
		y = ty;
		
		width  = twidth;
		height = theight;
	}
	
	RN_INLINE Rect::Rect(const Vector2& origin, float twidth, float theight)
	{
		x = origin.x;
		y = origin.y;
		
		width  = twidth;
		height = theight;
	}
	
	RN_INLINE Rect::Rect(const Vector2& origin, const Vector2& size)
	{
		x = origin.x;
		y = origin.y;
		
		width  = size.x;
		height = size.y;
	}
	
	RN_INLINE Rect::Rect(const Rect& other)
	{
		x = other.x;
		y = other.y;
		
		width  = other.width;
		height = other.height;
	}
	
	RN_INLINE bool Rect::operator== (const Rect& other)
	{
		if(abs(x - other.x) > k::EpsilonFloat || abs(y - other.y) > k::EpsilonFloat ||
		   abs(width - other.width) > k::EpsilonFloat || abs(height - other.height) > k::EpsilonFloat)
			return false;
		
		return true;
	}
	
	RN_INLINE bool Rect::operator!= (const Rect& other)
	{
		if(abs(x - other.x) > k::EpsilonFloat || abs(y - other.y) > k::EpsilonFloat ||
		   abs(width - other.width) > k::EpsilonFloat || abs(height - other.height) > k::EpsilonFloat)
			return true;
		
		return false;
	}
	
	
	RN_INLINE bool Rect::ContainsPoint(const Vector2& point) const
	{
		return ((point.x >= x && point.x <= x + width) && (point.y >= y && point.y <= y + height));
	}
	
	RN_INLINE bool Rect::IntersectsRect(const Rect& other) const
	{
		return ((x < other.x + other.width && x + width > other.x) &&
				(y < other.y + other.height && y + height > other.y));
	}
	
	RN_INLINE bool Rect::ContainsRect(const Rect& other) const
	{
		return ((x <= other.x && x + width >= other.x + other.width) && (y <= other.y && y + height >= other.y + other.height));
	}
	
	
	RN_INLINE Rect& Rect::Inset(float dx, float dy)
	{
		x += dx;
		y += dy;
		
		width  -= dx * 2;
		height -= dy * 2;
		
		return *this;
	}
	
	RN_INLINE Rect& Rect::Integral()
	{
		x = roundf(x);
		y = roundf(y);
		
		width  = roundf(width);
		height = roundf(height);
		
		return *this;
	}
	
	
	RN_INLINE float Rect::Top() const
	{
		return y + height;
	}
	
	RN_INLINE float Rect::Bottom() const
	{
		return y;
	}
	
	RN_INLINE float Rect::Left() const
	{
		return x;
	}
	
	RN_INLINE float Rect::Right() const
	{
		return x + width;
	}
}

#endif /* __RAYNE_RECT_H__ */
