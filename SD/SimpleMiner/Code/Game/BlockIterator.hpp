#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"

class Chunk;
struct Block;

struct BlockIterator
{
public:
	BlockIterator() {}
	BlockIterator(Chunk* chunk, int blockIndex);
	BlockIterator(Chunk* chunk, Vec3 const& worldPos);
	~BlockIterator() {}

	Block* GetBlock() const;
	BlockIterator GetEastNeighbor() const;
	BlockIterator GetWestNeighbor() const;
	BlockIterator GetNorthNeighbor() const;
	BlockIterator GetSouthNeighbor() const;
	BlockIterator GetTopNeighbor() const;
	BlockIterator GetBottomNeighbor() const;
	AABB3 GetBlockBounds() const;

	bool operator==(BlockIterator const& compare) const;
	bool operator!=(BlockIterator const& compare) const;

public:
	Chunk* m_chunk = nullptr;
	int m_blockIndex = 0;
};


