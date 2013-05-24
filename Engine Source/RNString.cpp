//
//  RNString.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include <codecvt>
#include <locale>

#include "RNString.h"

#define kRNStringExpansionSize 64

namespace RN
{
	RNDeclareMeta(String)
	
	// ---------------------
	// MARK: -
	// MARK: CodePoint
	// ---------------------
	
	class CodePoint
	{
	public:
		CodePoint() {}
		CodePoint(char character)
		{
			_codePoint = static_cast<UniChar>(character);
		}
		
		CodePoint(UniChar codepoint)
		{
			_codePoint = codepoint;
		}
		
		CodePoint& operator =(uint32 point)
		{
			_codePoint = point;
			return *this;
		}
		
		bool operator ==(const CodePoint& other) const
		{
			return (_codePoint == other._codePoint);
		}
		
		bool operator >(const CodePoint& other) const
		{
			return _codePoint > other._codePoint;
		}
		
		bool operator <(const CodePoint& other) const
		{
			return _codePoint < other._codePoint;
		}
		
		operator uint32()
		{
			return _codePoint;
		}
		
		UniChar LowerCase();
		UniChar UpperCase();
		
	private:
		UniChar _codePoint;
	};
	
	UniChar CodePoint::LowerCase()
	{
		if(_codePoint <= 0x7f)
		{
			char character = static_cast<char>(_codePoint);
			if(character >= 'A' && character <= 'Z')
			{
				character = tolower(character);
				return CodePoint(character);
			}
		}
		
		return *this;
	}
	
	UniChar CodePoint::UpperCase()
	{
		if(_codePoint <= 0x7f)
		{
			char character = static_cast<char>(_codePoint);
			if(character >= 'a' && character <= 'z')
			{
				character = toupper(character);
				return CodePoint(character);
			}
		}
		
		return *this;
	}
	
	// ---------------------
	// MARK: -
	// MARK: String
	// ---------------------
	
	static const char UTF8TrailingBytes[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
	};
	
	typedef std::codecvt_utf8_utf16<char16_t, 0x10ffff, std::little_endian> UTF16LEFacet;
	typedef std::codecvt_utf8_utf16<char16_t, 0x10ffff> UTF16BEFacet;
	
	
	String::String()
	{
		Initialize();
		AllocateBuffer(32);
	}
	
	String::String(const char *string)
	{
		Initialize();
		CopyBytesWithEncoding(string, strlen(string), Encoding::ASCII);
	}
	
	String::String(const char *string, size_t length)
	{
		Initialize();
		CopyBytesWithEncoding(string, length, Encoding::ASCII);
	}
	
	String::String(const void *bytes, Encoding encoding)
	{
		Initialize();
		
		switch(encoding)
		{
			case Encoding::ASCII:
			{
				size_t length = strlen(static_cast<const char *>(bytes));
				CopyBytesWithEncoding(bytes, length, Encoding::ASCII);
				break;
			}
				
			case Encoding::UTF8:
			{
				CopyBytesWithEncoding(bytes, UTF8ByteLength(static_cast<const uint8 *>(bytes)), Encoding::UTF8);
				break;
			}
	
			case Encoding::UTF16LE:
			case Encoding::UTF16BE:
			{
				std::string converted;
				
				if(encoding == Encoding::UTF16LE)
				{
					std::wstring_convert<UTF16LEFacet, char16_t> convert;
					converted = convert.to_bytes(static_cast<const char16_t *>(bytes));
				}
				else
				{
					std::wstring_convert<UTF16BEFacet, char16_t> convert;
					converted = convert.to_bytes(static_cast<const char16_t *>(bytes));
				}
				
				const uint8 *data = (const uint8 *)converted.data();
				CopyBytesWithEncoding(data, UTF8ByteLength(data), Encoding::UTF8);
				break;
			}
		}
	}
	
	String::String(const void *bytes, size_t length, Encoding encoding)
	{
		Initialize();
		CopyBytesWithEncoding(bytes, length, encoding);
	}
	
	String::String(const String& string)
	{
		Initialize();
		
		_buffer = new uint8[string._size];
		_length = string._length;
		_size = string._size;
		_occupied = string._occupied;
		
		std::copy(string._buffer, string._buffer + _length + 1, _buffer);
	}
	
	String::~String()
	{
		delete [] _buffer;
	}
	
	
	
	
	
	void String::Initialize()
	{
		_buffer = 0;
		_length = 0;
		_size = 0;
		_occupied = 0;
	}
	
	// ---------------------
	// MARK: -
	// MARK: Low Level handling
	// ---------------------
	
	void String::AllocateBuffer(size_t size)
	{
		uint8 *temp = new uint8[size];
		if(!temp)
			throw ErrorException(0, 0, 0);
		
		if(_buffer)
		{
			size_t copy = MIN(size, _occupied);
			std::copy(_buffer, _buffer + copy, temp);
			
			delete [] _buffer;
		}
		
		_buffer = temp;
		_size = static_cast<uint32>(size);
	}
	
	void String::CheckAndExpandBuffer(uint32 minium)
	{
		if(_occupied + minium > _size)
		{
			size_t newSize = _occupied + minium + kRNStringExpansionSize;
			if(_size == 0)
				newSize = minium;
			
			AllocateBuffer(newSize);
		}
	}
	
	bool String::IsLegalUTF8(const uint8 *sequence, int length) const
	{
		const uint8 *end = sequence + length;
		uint8 character;
		
		switch(length)
		{
			case 4:
				if((character = (*--end)) < 0x80 || character > 0xBF)
					return false;
			case 3:
				if((character = (*--end)) < 0x80 || character > 0xBF)
					return false;
			case 2:
				if((character = (*--end)) > 0xBF)
					return false;
				
			switch(*sequence)
			{
				case 0xe0:
					if(character < 0xa0)
						return false;
					break;
					
				case 0xed:
					if(character > 0x9f)
						return false;
					break;
					
				case 0xf0:
					if(character< 0x90)
						return false;
					break;
					
				case 0xf4:
					if(character > 0x8f)
						return false;
					break;
						
				default:
					if(character < 0x80)
						return false;
					break;
			}
				
			case 1:
				if(*sequence >= 0x80 && *sequence < 0xc2)
					return false;
				break;
				
			default:
				return false;
		}
		
		if(*sequence > 0xf4)
			return false;
		
		return true;
	}
	
	size_t String::UTF8ByteLength(const uint8 *bytes) const
	{
		size_t length = 0;
		
		while(*bytes != '\0')
		{
			size_t cbytes = (UTF8TrailingBytes[*bytes] + 1);
			
			bytes += cbytes;
			length += cbytes;
		}
		
		return length;
	}
	
	void String::CopyUTF8Bytes(const uint8 *bytes, size_t length)
	{
		CheckAndExpandBuffer(static_cast<uint32>(length) + 1);
		
		uint32 characters = 0;
		const uint8 *end = bytes + length;
		
		for(size_t i=0; i<length;)
		{
			int trailing = UTF8TrailingBytes[bytes[i]];
			
			if(bytes + i + trailing >= end)
				break;
			
			if(*(bytes + i) == 0)
				break;
			
			if(IsLegalUTF8(bytes + i, trailing + 1))
			{
				std::copy(bytes + i, bytes + (i + trailing + 1), _buffer + _occupied);
				_occupied += trailing + 1;
				
				characters ++;
			}
			else
			{
				throw ErrorException(0, 0, 0);
			}
			
			i += trailing + 1;
		}
		
		_buffer[_occupied] = '\0';
		_length += characters;
	}
	
	void String::CopyBytesWithEncoding(const void *bytes, size_t length, Encoding encoding)
	{
		CheckAndExpandBuffer(static_cast<uint32>(length) + 1);
		
		switch(encoding)
		{
			case Encoding::ASCII:
			{
				const uint8 *data = static_cast<const uint8 *>(bytes);
				std::copy(data, data + length, _buffer + _occupied);
				
				_length += static_cast<uint32>(length);
				_occupied += static_cast<uint32>(length);
				
				_buffer[_occupied] = '\0';
				break;
			}
				
			case Encoding::UTF8:
			{
				const uint8 *tbytes = static_cast<const uint8 *>(bytes);
				CopyUTF8Bytes(tbytes, length);
				break;
			}
				
			case Encoding::UTF16LE:
			case Encoding::UTF16BE:
			{
				std::string converted;
				
				if(encoding == Encoding::UTF16LE)
				{
					std::wstring_convert<UTF16LEFacet, char16_t> convert;
					converted = convert.to_bytes(static_cast<const char16_t *>(bytes));
				}
				else
				{
					std::wstring_convert<UTF16BEFacet, char16_t> convert;
					converted = convert.to_bytes(static_cast<const char16_t *>(bytes));
				}
				
				const uint8 *data = (const uint8 *)converted.data();
				CopyUTF8Bytes(data, length);
				break;
			}
		}
	}
	
	// ---------------------
	// MARK: -
	// MARK: Operator
	// ---------------------
	
	bool String::operator ==(const String& other) const
	{
		return (Compare(other) == kRNCompareEqualTo);
	}
	
	bool String::operator !=(const String& other) const
	{
		return (Compare(other) != kRNCompareEqualTo);
	}
	
	String& String::operator +=(const String& other)
	{
		Append(other);
		return *this;
	}
	
	String String::operator +(const String& other) const
	{
		String result(*this);
		result += other;
		
		return result;
	}
	
	// ---------------------
	// MARK: -
	// MARK: Mutation
	// ---------------------
	
	void String::Append(const String& string)
	{
		CopyUTF8Bytes(string._buffer, string._occupied);
	}
	
	void String::Append(const String *string)
	{
		CopyUTF8Bytes(string->_buffer, string->_occupied);
	}
	
	
	void String::Insert(const String& string, uint32 index)
	{
		ReplaceCharacters(string, Range(index, 0));
	}
	
	void String::Insert(const String *string, uint32 index)
	{
		ReplaceCharacters(string, Range(index, 0));
	}
	
	
	void String::DeleteCharacters(const Range& range)
	{
		ReplaceCharacters(String(), range);
	}
	
	
	void String::ReplaceCharacters(const String& replacement, const Range& range)
	{
		ReplaceCharacters(&replacement, range);
	}
	
	void String::ReplaceCharacters(const String *replacement, const Range& range)
	{
		CheckAndExpandBuffer(replacement->_length);
		
		uint8 *data = _buffer;
		uint8 *dataEnd = _buffer + _occupied;
		
		RN_ASSERT0(range.origin + range.length <= _length);
		
		uint32 intialOccupied = 0;
		uint32 appendixLength = 0;
		
		// Skip the first n characters
		for(machine_uint i=0; i<range.origin; i++)
		{
			size_t length = UTF8TrailingBytes[*data] + 1;
			data += length;
			intialOccupied += length;
			appendixLength ++;
			
			if(data > dataEnd)
				throw ErrorException(0, 0, 0);
		}
		
		uint8 *begin = data;
		uint8 *end = data;
		
		// Find the beginning of the end
		for(machine_uint i=0; i<range.length; i++)
		{
			size_t length = UTF8TrailingBytes[*end] + 1;
			end += length;
			appendixLength ++; 
			
			if(end > dataEnd)
				throw ErrorException(0, 0, 0);
		}
		
		appendixLength = (_length - appendixLength);
		
		// Copy the appendix of this string into a temporary buffer
		size_t bytes = dataEnd - end;
		uint8 *temp = 0;
		
		if(bytes > 0)
		{
			temp = new uint8[bytes];
			std::copy(end, dataEnd, temp);
		}
		
		// Copy everything back together
		_length   = static_cast<uint32>(range.origin);
		_occupied = intialOccupied;
		
		if(replacement->_length > 0)
		{
			uint32 rSize = replacement->_occupied;
			std::copy(replacement->_buffer, replacement->_buffer + rSize, begin);
			
			_length   += replacement->_length;
			_occupied += rSize;
			
			begin += rSize;
		}
		
		if(temp)
		{
			_length   += appendixLength;
			_occupied += bytes;
			
			std::copy(temp, temp + bytes, begin);
			delete [] temp;
		}
		
		_buffer[_occupied] = '\0';
	}
	
	void String::ReplaceOccurrencesOfString(const String& string, const String& replacement)
	{
		ReplaceOccurrencesOfString(&string, &replacement);
	}
	
	void String::ReplaceOccurrencesOfString(const String *string, const String *replacement)
	{
		while(1)
		{
			Range range = RangeOfString(string);
			if(range.origin == kRNNotFound)
				break;
			
			ReplaceCharacters(replacement, range);
		}
	}
	
	// ---------------------
	// MARK: -
	// MARK: Comparison
	// ---------------------
	
#define PeekCharacter(point, string) do { \
		uint8 temp[5] = { 0 }; \
		size_t length = UTF8TrailingBytes[*(string)] + 1; \
		\
		std::copy(string, string + length, temp); \
		std::basic_string<char32_t> converted = convert.from_bytes((const char *)temp); \
		\
		point = CodePoint(static_cast<uint32>(converted[0])); \
		string += length; \
	} while(0)
	
	Range String::RangeOfString(const String& string)
	{
		return RangeOfString(&string, 0, Range(0, _length));
	}
	
	Range String::RangeOfString(const String& string, ComparisonMode mode)
	{
		return RangeOfString(&string, mode, Range(0, _length));
	}
	
	Range String::RangeOfString(const String& string, ComparisonMode mode, const Range& range)
	{
		return RangeOfString(&string, mode, range);
	}
	
	
	
	Range String::RangeOfString(const String *string)
	{
		return RangeOfString(string, 0, Range(0, _length));
	}
	
	Range String::RangeOfString(const String *string, ComparisonMode mode)
	{
		return RangeOfString(string, mode, Range(0, _length));
	}
	
	Range String::RangeOfString(const String *string, ComparisonMode mode, const Range& range)
	{
		const uint8 *data = _buffer;
		const uint8 *dataEnd = _buffer + _occupied;
		
		const uint8 *stringData = string->_buffer;
		const uint8 *stringDataTemp;
		
		RN_ASSERT0(range.origin + range.length <= _length);
		
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
		
		CodePoint first;
		PeekCharacter(first, stringData);
		
		stringDataTemp = stringData;
		
		// Skip the first n characters
		for(machine_uint i=0; i<range.origin; i++)
		{
			size_t length;
			
			length = UTF8TrailingBytes[*data] + 1;
			data += length;
			
			if(data > dataEnd)
				throw ErrorException(0, 0, 0);
		}
		
		// Calculate the new end
		do {
			const uint8 *temp = data;
			
			for(machine_uint i=0; i<range.length; i++)
			{
				size_t length;
				
				length = UTF8TrailingBytes[*temp] + 1;
				temp += length;
				
				if(temp > dataEnd)
					throw ErrorException(0, 0, 0);
			}
			
			dataEnd = temp;
		} while(0);
		
		Range result = Range(kRNNotFound, 0);
		machine_uint index = range.origin;
		
		while(data < dataEnd)
		{
			CodePoint point;
			PeekCharacter(point, data);
			
			if(result.origin == kRNNotFound)
			{
				if(point == first)
				{
					result.origin = index;
					result.length = 1;
				}
			}
			else
			{
				CodePoint read;
				PeekCharacter(read, stringData);
				
				if(mode & ComparisonModeCaseInsensitive)
				{
					read  = read.LowerCase();
					point = point.LowerCase();
				}
				
				if(read != point)
				{
					stringData = stringDataTemp;
					result = Range(kRNNotFound, 0);
				}
				else
				{
					result.length ++;
					
					if(result.length >= string->_length)
						break;
				}
			}
			
			index ++;
		}
		
		return result;
	}
	
	
	ComparisonResult String::Compare(const String& other) const
	{
		return Compare(&other, 0, Range(0, _length));
	}
	
	ComparisonResult String::Compare(const String& other, ComparisonMode mode) const
	{
		return Compare(&other, mode, Range(0, _length));
	}
	
	ComparisonResult String::Compare(const String& other, ComparisonMode mode, const Range& range) const
	{
		return Compare(&other, mode, range);
	}
	
	
	ComparisonResult String::Compare(const String *other) const
	{
		return Compare(other, 0, Range(0, _length));
	}
	
	ComparisonResult String::Compare(const String *other, ComparisonMode mode) const
	{
		return Compare(other, mode, Range(0, _length));
	}
	
	ComparisonResult String::Compare(const String *other, ComparisonMode mode, const Range& range) const
	{
		const uint8 *data = _buffer;
		const uint8 *dataEnd = _buffer + _occupied;
		
		const uint8 *compareData = other->_buffer;
		const uint8 *compareDataEnd = other->_buffer + other->_occupied;
		
		RN_ASSERT0(range.origin + range.length <= _length);
		
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
		
		// Skip the first n characters
		for(machine_uint i=0; i<range.origin; i++)
		{
			size_t length = UTF8TrailingBytes[*data] + 1;
			data += length;
			
			if(data > dataEnd)
				throw ErrorException(0, 0, 0);
		}
		
		// Calculate the new end
		do {
			const uint8 *temp = data;
			
			for(machine_uint i=0; i<range.length; i++)
			{
				size_t length = UTF8TrailingBytes[*temp] + 1;
				temp += length;

				if(temp > dataEnd)
					throw ErrorException(0, 0, 0);
			}
			
			dataEnd = temp;
		} while(0);

		while(data < dataEnd && compareData < compareDataEnd)
		{
			CodePoint a, b;
			
			PeekCharacter(a, data);
			PeekCharacter(b, compareData);
			
			if(mode & ComparisonModeCaseInsensitive)
			{
				a = a.LowerCase();
				b = b.LowerCase();
			}
			
			if(mode & ComparisonModeNumerically)
			{
				if(a <= 0x7f && b <= 0x7f)
				{
					char ca = static_cast<char>(a);
					char cb = static_cast<char>(b);
					
					if(ca <= '9' && ca >= '0' && cb <= '9' && cb >= '0')
					{
						uint32 numA = 0;
						uint32 numB = 0;
						
						do {
							char ca = static_cast<char>(a);
							numA = numA * 10 + (ca - '0');
							
							PeekCharacter(a, data);
						} while(ca <= '9' && ca >= '0' && data < dataEnd);
						
						do {
							char cb = static_cast<char>(b);
							numB = numB * 10 + (cb - '0');
							
							PeekCharacter(b, compareData);
						} while(cb <= '9' && cb >= '0' && compareData < compareDataEnd);
						
						if(numA > numB)
							return kRNCompareGreaterThan;
						
						if(numB > numA)
							return kRNCompareLessThan;
						
						continue;
					}
				}
			}
			
			if(a != b)
			{
				if(a > b)
					return kRNCompareGreaterThan;
				
				if(b > a)
					return kRNCompareLessThan;
			}
		}
		
		return kRNCompareEqualTo;
	}
	
#undef PeekCharacter
	
	// ---------------------
	// MARK: -
	// MARK: Conversion / Access
	// ---------------------
	
	String String::Substring(const Range& range) const
	{
		const uint8 *data = _buffer;
		const uint8 *dataEnd = _buffer + _occupied;
		
		for(machine_uint i=0; i<range.origin; i++)
		{
			size_t length = UTF8TrailingBytes[*data] + 1;
			data += length;
			
			if(data > dataEnd)
				throw ErrorException(0, 0, 0);
		}
		
		size_t bytes = 0;
		const uint8 *temp = data;
		
		for(machine_uint i=0; i<range.length; i++)
		{
			size_t length = UTF8TrailingBytes[*temp] + 1;
			temp  += length;
			bytes += length;
			
			if(temp > dataEnd)
				throw ErrorException(0, 0, 0);
		}
		
		return String(data, bytes, Encoding::UTF8);
	}
	
	UniChar String::CharacterAtIndex(uint32 index) const
	{
		uint8 *data = _buffer;
		
		for(uint32 i=0; i<_length; i++)
		{
			if(i == index)
			{
				uint8 temp[5] = { 0 };
				size_t length = UTF8TrailingBytes[*data] + 1;
				
				std::copy(data, data + length, temp);
				
				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				std::basic_string<char32_t> converted = convert.from_bytes((const char *)temp);
				
				UniChar character = static_cast<UniChar>(converted[0]);
				return character;
			}
			
			size_t length = UTF8TrailingBytes[*data] + 1;
			data += length;
		}
		
		throw ErrorException(0, 0, 0);
	}
	
	uint8 *String::BytesWithEncoding(Encoding encoding, bool lossy, size_t *outLength) const
	{
		switch(encoding)
		{
			case Encoding::ASCII:
			{
				char *buffer = new char[_length + 1];
				const uint8 *temp = _buffer;
				
				for(uint32 i=0; i<_length; i++)
				{
					size_t length = UTF8TrailingBytes[*temp] + 1;
					
					if((*temp > 0x7f || length > 1) && !lossy)
					{
						delete [] buffer;
						return 0;
					}
					
					buffer[i] = (*temp <= 0x7f) ? static_cast<char>(*temp) : '?';
					temp += length;
				}
				
				buffer[_length] = '\0';
				
				if(outLength)
					*outLength = _length;
				
				return reinterpret_cast<uint8 *>(buffer);
				break;
			}
				
			case Encoding::UTF8:
			{
				uint8 *buffer = new uint8[_occupied];
				std::copy(_buffer, _buffer + _occupied, buffer);
				buffer[_occupied] = '\0';
				
				if(outLength)
					*outLength = _occupied;
				
				return buffer;
				break;
			}
				
			case Encoding::UTF16BE:
			case Encoding::UTF16LE:
			{
				std::basic_string<char16_t> string;
				
				if(encoding == Encoding::UTF16LE)
				{
					std::wstring_convert<UTF16LEFacet, char16_t> convert;
					string = convert.from_bytes(reinterpret_cast<char *>(_buffer));
				}
				else
				{
					std::wstring_convert<UTF16BEFacet, char16_t> convert;
					string = convert.from_bytes(reinterpret_cast<char *>(_buffer));
				}
				
				char16_t *buffer = new char16_t[string.size() + 1];
				const char16_t *data = string.data();
				
				std::copy(data, data + string.size(), buffer);
				buffer[string.size()] = '\0';
				
				if(outLength)
					*outLength = string.size() * sizeof(char16_t);
				
				return reinterpret_cast<uint8 *>(buffer);
				break;
			}
		}
		
		throw ErrorException(0);
	}
}