#include "Engine/Core/BufferUtils.hpp"

#include <utility>
//
// buffer writer
//

BufferWriter::BufferWriter(std::vector<unsigned char>& buffer)
	:m_buffer(buffer)
{
	m_initialBufferSize = (int)m_buffer.size();
	m_nativeEndianess = GetNativeEndianess();
	m_endianMode = m_nativeEndianess;
}


void BufferWriter::SetEndianMode(eBufferEndian endianMode)
{
	m_endianMode = endianMode;
	m_isOppositeEndianess = (m_endianMode != m_nativeEndianess);
}


void BufferWriter::AppendChar(char data)
{
	m_buffer.push_back((unsigned char)data);
}


void BufferWriter::AppendByte(unsigned char data)
{
	m_buffer.push_back(data);
}


void BufferWriter::AppendShort(short data)
{
	AppendTwoBytes((unsigned short&)data);
}


void BufferWriter::AppendUShort(unsigned short data)
{
	AppendTwoBytes(data);
}


void BufferWriter::AppendInt32(int32_t data)
{
	AppendFourBytes((uint32_t&)data);
}


void BufferWriter::AppendUint32(uint32_t data)
{
	AppendFourBytes(data);
}


void BufferWriter::AppendInt64(int64_t data)
{
	AppendEightBytes((uint64_t&)data);
}


void BufferWriter::AppendUint64(uint64_t data)
{
	AppendEightBytes(data);
}


void BufferWriter::AppendFloat(float data)
{
	AppendFourBytes((uint32_t&)data);
}


void BufferWriter::AppendDouble(double data)
{
	AppendEightBytes((uint64_t&)data);
}


void BufferWriter::AppendBool(bool data)
{
	unsigned char input = data ? 1 : 0;
	m_buffer.push_back(input);
}


void BufferWriter::AppendStringZeroTerminated(std::string const& data)
{
	for (int index = 0; index < (int)data.size(); index++)
	{
		m_buffer.push_back(data[index]);
	}
	m_buffer.push_back('\0');
}


void BufferWriter::AppendStringAfter32BitLength(std::string const& data)
{
	uint32_t size = (uint32_t)data.size();
	AppendUint32(size);
	for (int index = 0; index < (int)size; index++)
	{
		m_buffer.push_back(data[index]);
	}
}


void BufferWriter::AppendVec2(Vec2 const& data)
{
	AppendFloat(data.x);
	AppendFloat(data.y);
}


void BufferWriter::AppendVec3(Vec3 const& data)
{
	AppendFloat(data.x);
	AppendFloat(data.y);
	AppendFloat(data.z);
}


void BufferWriter::AppendIntVec2(IntVec2 const& data)
{
	AppendInt32(data.x);
	AppendInt32(data.y);
}


void BufferWriter::AppendIntVec3(IntVec3 const& data)
{
	AppendInt32(data.x);
	AppendInt32(data.y);
	AppendInt32(data.z);
}


void BufferWriter::AppendRgb8(Rgba8 const& data)
{
	AppendByte(data.r);
	AppendByte(data.g);
	AppendByte(data.b);
}


void BufferWriter::AppendRgba8(Rgba8 const& data)
{
	AppendByte(data.r);
	AppendByte(data.g);
	AppendByte(data.b);
	AppendByte(data.a);
}


void BufferWriter::AppendVertexPCU(Vertex_PCU const& data)
{
	AppendVec3(data.m_position);
	AppendRgba8(data.m_color);
	AppendVec2(data.m_uvTexCoords);
}


void BufferWriter::UpdateUInt32AtOffset(uint32_t& data, uint32_t offset)
{
	if (m_isOppositeEndianess)
	{
		SwapFourBytesInPlace(data);
	}
	unsigned char* value = (unsigned char*) &data;
	m_buffer[offset] = value[0];
	m_buffer[(size_t)offset + 1] = value[1];
	m_buffer[(size_t)offset + 2] = value[2];
	m_buffer[(size_t)offset + 3] = value[3];
}


int BufferWriter::GetAppendedSize() const
{
	return (int)m_buffer.size() - m_initialBufferSize;
}


int BufferWriter::GetTotalSize() const
{
	return (int)m_buffer.size();
}


eBufferEndian BufferWriter::GetEndianMode() const
{
	return m_endianMode;
}


void BufferWriter::AppendTwoBytes(unsigned short& data)
{
	if (m_isOppositeEndianess)
	{
		SwapTwoBytesInPlace(data);
	}
	unsigned char* value = (unsigned char*) &data;
	m_buffer.push_back(value[0]);
	m_buffer.push_back(value[1]);
}


void BufferWriter::AppendFourBytes(uint32_t& data)
{
	if (m_isOppositeEndianess)
	{
		SwapFourBytesInPlace(data);
	}
	unsigned char* value = (unsigned char*) &data;
	m_buffer.push_back(value[0]);
	m_buffer.push_back(value[1]);
	m_buffer.push_back(value[2]);
	m_buffer.push_back(value[3]);
}


void BufferWriter::AppendEightBytes(uint64_t& data)
{
	if (m_isOppositeEndianess)
	{
		SwapEightBytesInPlace(data);
	}
	unsigned char* value = (unsigned char*) &data;
	m_buffer.push_back(value[0]);
	m_buffer.push_back(value[1]);
	m_buffer.push_back(value[2]);
	m_buffer.push_back(value[3]);
	m_buffer.push_back(value[4]);
	m_buffer.push_back(value[5]);
	m_buffer.push_back(value[6]);
	m_buffer.push_back(value[7]);
}


//
//	buffer parser
//

BufferParser::BufferParser(std::vector<unsigned char>& buffer)
	:m_buffer(buffer)
{
	m_currentOffset = 0;
	m_nativeEndianess = GetNativeEndianess();
	m_endianMode = m_nativeEndianess;
}


void BufferParser::SetEndianMode(eBufferEndian endianMode)
{
	m_endianMode = endianMode;
	m_isOppositeEndianess = (m_endianMode != m_nativeEndianess);
}


char BufferParser::ParseChar()
{
	char out = (char)m_buffer[m_currentOffset];
	m_currentOffset++;
	return out;
}


unsigned char BufferParser::ParseByte()
{
	unsigned char out = m_buffer[m_currentOffset];
	m_currentOffset++;
	return out;
}


short BufferParser::ParseShort()
{
	return (short)ParseTwoBytes();
}


unsigned short BufferParser::ParseUShort()
{
	return ParseTwoBytes();
}


int32_t BufferParser::ParseInt32()
{
	return (int32_t)ParseFourBytes();
}


uint32_t BufferParser::ParseUint32()
{
	return ParseFourBytes();
}


int64_t BufferParser::ParseInt64()
{
	return (int64_t)ParseEightBytes();
}


uint64_t BufferParser::ParseUint64()
{
	return ParseEightBytes();
}


float BufferParser::ParseFloat()
{
	uint32_t out = ParseFourBytes();
	return *reinterpret_cast<float*>(&out);
}


double BufferParser::ParseDouble()
{
	uint64_t out = ParseEightBytes();
	return *reinterpret_cast<double*>(&out);
}


bool BufferParser::ParseBool()
{
	char out = (char)m_buffer[m_currentOffset];
	m_currentOffset++;
	return out ? true : false;
}


std::string BufferParser::ParseStringZeroTerminated()
{
	std::string out = "";
	while (m_buffer[m_currentOffset] != '\0')
	{
		out.push_back(m_buffer[m_currentOffset]);
		m_currentOffset++;
	}
	m_currentOffset++;
	return out;
}


std::string BufferParser::ParseStringAfter32BitLength()
{
	uint32_t size = ParseUint32();
	std::string out;
	out.assign(&m_buffer[m_currentOffset], &m_buffer[m_currentOffset] + size);
	m_currentOffset += size;
	return out;
}


Vec2 BufferParser::ParseVec2()
{
	float x = ParseFloat();
	float y = ParseFloat();
	return Vec2(x, y);
}


Vec3 BufferParser::ParseVec3()
{
	float x = ParseFloat();
	float y = ParseFloat();
	float z = ParseFloat();
	return Vec3(x, y, z);
}


IntVec2 BufferParser::ParseIntVec2()
{
	int x = ParseInt32();
	int y = ParseInt32();
	return IntVec2(x, y);
}


IntVec3 BufferParser::ParseIntVec3()
{
	int x = ParseInt32();
	int y = ParseInt32();
	int z = ParseInt32();
	return IntVec3(x, y, z);
}


Rgba8 BufferParser::ParseRgb8()
{
	unsigned char r = ParseByte();
	unsigned char g = ParseByte();
	unsigned char b = ParseByte();
	return Rgba8(r, g, b, 255);
}


Rgba8 BufferParser::ParseRgba8()
{
	unsigned char r = ParseByte();
	unsigned char g = ParseByte();
	unsigned char b = ParseByte();
	unsigned char a = ParseByte();
	return Rgba8(r, g, b, a);
}


Vertex_PCU BufferParser::ParseVertexPCU()
{
	Vec3 position = ParseVec3();
	Rgba8 color = ParseRgba8();
	Vec2 uvs = ParseVec2();
	return Vertex_PCU(position, color, uvs);
}


void BufferParser::SetCurrentOffset(uint32_t offset)
{
	m_currentOffset = offset;
}


int BufferParser::GetRemainingSize() const
{
	return (int)m_buffer.size() - m_currentOffset;
}


unsigned short BufferParser::ParseTwoBytes()
{
	unsigned short out;
	memcpy(&out, &m_buffer[m_currentOffset], sizeof(unsigned short));
	m_currentOffset += 2;
	if (m_isOppositeEndianess)
	{
		SwapTwoBytesInPlace(out);
	}
	return out;
}


uint32_t BufferParser::ParseFourBytes()
{
	uint32_t out;
	memcpy(&out, &m_buffer[m_currentOffset], sizeof(uint32_t));
	m_currentOffset += 4;
	if (m_isOppositeEndianess)
	{
		SwapFourBytesInPlace(out);
	}
	return out;
}


uint64_t BufferParser::ParseEightBytes()
{
	uint64_t out;
	memcpy(&out, &m_buffer[m_currentOffset], sizeof(uint64_t));
	m_currentOffset += 8;
	if (m_isOppositeEndianess)
	{
		SwapEightBytesInPlace(out);
	}
	return out;
}


eBufferEndian GetNativeEndianess()
{
	union IntChars
	{
		int value = 0x12345678;
		unsigned char firstByte;
	};

	IntChars validater;

	if (validater.firstByte == 0x78)
	{
		return eBufferEndian::LITTLE;
	}
	else
	{
		return eBufferEndian::BIG;
	}
}


void SwapTwoBytesInPlace(unsigned short& data)
{
	unsigned char* value = (unsigned char*)&data;
	std::swap(value[0], value[1]);
}


void SwapFourBytesInPlace(uint32_t& data)
{
	unsigned char* value = (unsigned char*)&data;
	std::swap(value[0], value[3]);
	std::swap(value[1], value[2]);
}


void SwapEightBytesInPlace(uint64_t& data)
{
	unsigned char* value = (unsigned char*)&data;
	std::swap(value[0], value[7]);
	std::swap(value[1], value[6]);
	std::swap(value[2], value[5]);
	std::swap(value[3], value[4]);
}


