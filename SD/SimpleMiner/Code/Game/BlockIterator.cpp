#include "Game/BlockIterator.hpp"
#include "Game/Chunk.hpp"
#include "Game/Block.hpp"

BlockIterator::BlockIterator(Chunk* chunk, int blockIndex)
	: m_chunk(chunk)
	, m_blockIndex(blockIndex)
{
}


BlockIterator::BlockIterator(Chunk* chunk, Vec3 const& worldPos)
	: m_chunk(chunk)
{
	int x = RoundDownToInt(worldPos.x);
	int y = RoundDownToInt(worldPos.y);
	int z = RoundDownToInt(worldPos.z);
	m_blockIndex = m_chunk->GetIndexForWorldCoords(IntVec3(x, y, z));
}


Block* BlockIterator::GetBlock() const
{
	if (m_chunk && m_blockIndex >= 0 && m_blockIndex < CHUNK_BLOCKS_TOTAL)
	{
		return &(m_chunk->m_blocks[m_blockIndex]);
	}
	
	return nullptr;
}


BlockIterator BlockIterator::GetEastNeighbor() const
{
	if (m_chunk && ((m_blockIndex & CHUNK_MASK_X) == CHUNK_MASK_X))
	{
		return BlockIterator(m_chunk->m_eastChunk, m_blockIndex & ~CHUNK_MASK_X);
	}
	else
	{
		return BlockIterator(m_chunk, m_blockIndex + 1);
	}
}


BlockIterator BlockIterator::GetWestNeighbor() const
{
	if (m_chunk && ((m_blockIndex & CHUNK_MASK_X) == 0))
	{
		return BlockIterator(m_chunk->m_westChunk, m_blockIndex | CHUNK_MASK_X);
	}
	else
	{
		return BlockIterator(m_chunk, m_blockIndex - 1);
	}
}


BlockIterator BlockIterator::GetNorthNeighbor() const
{
	if (m_chunk && (((m_blockIndex >> CHUNK_BITSHIFT_Y) & CHUNK_MASK_Y) == CHUNK_MASK_Y))
	{
		return BlockIterator(m_chunk->m_northChunk, m_blockIndex & ~(CHUNK_MASK_Y << CHUNK_BITSHIFT_Y));
	}
	else
	{
		return BlockIterator(m_chunk, m_blockIndex + CHUNK_SIZE_X);
	}
}


BlockIterator BlockIterator::GetSouthNeighbor() const
{
	if (m_chunk && (((m_blockIndex >> CHUNK_BITSHIFT_Y) & CHUNK_MASK_Y) == 0))
	{
		return BlockIterator(m_chunk->m_southChunk, m_blockIndex | (CHUNK_MASK_Y << CHUNK_BITSHIFT_Y));
	}
	else
	{
		return BlockIterator(m_chunk, m_blockIndex - CHUNK_SIZE_X);
	}
}


BlockIterator BlockIterator::GetTopNeighbor() const
{
	return BlockIterator(m_chunk, m_blockIndex + CHUNK_BLOCKS_PER_LAYER);
}


BlockIterator BlockIterator::GetBottomNeighbor() const
{
	return BlockIterator(m_chunk, m_blockIndex - CHUNK_BLOCKS_PER_LAYER);
}


AABB3 BlockIterator::GetBlockBounds() const
{
	IntVec3 coords = m_chunk->GetWorldCoordsForIndex(m_blockIndex);
	return AABB3(Vec3(coords), Vec3(coords) + Vec3::ONE);
}


bool BlockIterator::operator==(BlockIterator const& compare) const
{
	return m_chunk == compare.m_chunk && m_blockIndex == compare.m_blockIndex;
}


bool BlockIterator::operator!=(BlockIterator const& compare) const
{
	return m_chunk != compare.m_chunk || m_blockIndex != compare.m_blockIndex;
}


