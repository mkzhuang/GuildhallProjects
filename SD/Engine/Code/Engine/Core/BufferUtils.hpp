#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"

#include <vector>
#include <string>

enum class eBufferEndian
{
	NATIVE,
	LITTLE,
	BIG,
};

class BufferWriter
{
public:
	BufferWriter(std::vector<unsigned char>& buffer);
	~BufferWriter() {}

	void SetEndianMode(eBufferEndian endianMode);
	void AppendChar(char data);
	void AppendByte(unsigned char data);
	void AppendShort(short data);
	void AppendUShort(unsigned short data);
	void AppendInt32(int32_t data);
	void AppendUint32(uint32_t data);
	void AppendInt64(int64_t data);
	void AppendUint64(uint64_t data);
	void AppendFloat(float data);
	void AppendDouble(double data);
	void AppendBool(bool data);
	void AppendStringZeroTerminated(std::string const& data);
	void AppendStringAfter32BitLength(std::string const& data);
	void AppendVec2(Vec2 const& data);
	void AppendVec3(Vec3 const& data);
	void AppendIntVec2(IntVec2 const& data);
	void AppendIntVec3(IntVec3 const& data);
	void AppendRgb8(Rgba8 const& data);
	void AppendRgba8(Rgba8 const& data);
	void AppendVertexPCU(Vertex_PCU const& data);

	void UpdateUInt32AtOffset(uint32_t& data, uint32_t offset);
	int GetAppendedSize() const;
	int GetTotalSize() const;
	eBufferEndian GetEndianMode() const;

private:
	void AppendTwoBytes(unsigned short& data);
	void AppendFourBytes(uint32_t& data);
	void AppendEightBytes(uint64_t& data);

protected:
	std::vector<unsigned char>& m_buffer;
	eBufferEndian m_nativeEndianess = eBufferEndian::NATIVE;
	eBufferEndian m_endianMode = eBufferEndian::NATIVE;
	bool m_isOppositeEndianess = false;
	int m_initialBufferSize = 0;
};

class BufferParser
{
public:
	BufferParser(std::vector<unsigned char>& buffer);
	~BufferParser() {}
	
	void			SetEndianMode(eBufferEndian endianMode);
	char			ParseChar();
	unsigned char	ParseByte();
	short			ParseShort();
	unsigned short	ParseUShort();
	int32_t			ParseInt32();
	uint32_t		ParseUint32();
	int64_t			ParseInt64();
	uint64_t		ParseUint64();
	float			ParseFloat();
	double			ParseDouble();
	bool			ParseBool();
	std::string		ParseStringZeroTerminated();
	std::string		ParseStringAfter32BitLength();
	Vec2			ParseVec2();
	Vec3			ParseVec3();
	IntVec2			ParseIntVec2();
	IntVec3			ParseIntVec3();
	Rgba8			ParseRgb8();
	Rgba8			ParseRgba8();
	Vertex_PCU		ParseVertexPCU();

	void SetCurrentOffset(uint32_t offset);
	int GetRemainingSize() const;

private:
	unsigned short ParseTwoBytes();
	uint32_t ParseFourBytes();
	uint64_t ParseEightBytes();

protected:
	std::vector<unsigned char>& m_buffer;
	uint32_t m_currentOffset = 0;
	eBufferEndian m_nativeEndianess = eBufferEndian::NATIVE;
	eBufferEndian m_endianMode = eBufferEndian::NATIVE;
	bool m_isOppositeEndianess = false;
};


eBufferEndian GetNativeEndianess();
void SwapTwoBytesInPlace(unsigned short& data);
void SwapFourBytesInPlace(uint32_t& data);
void SwapEightBytesInPlace(uint64_t& data);


