//
//  RNIndexPath.cpp
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNIndexPath.h"

namespace RN
{
	RNDefineMeta(IndexPath, Object)
	
	IndexPath::IndexPath()
	{}
	
	IndexPath::IndexPath(IndexPath *other)
	{
		_indices = other->_indices;
	}
	
	
	void IndexPath::AddIndex(size_t index)
	{
		_indices.push_back(index);
	}
	
	size_t IndexPath::GetLength() const
	{
		return _indices.size();
	}
	
	size_t IndexPath::GetIndexAtPosition(size_t position) const
	{
		return _indices.at(position);
	}
}
