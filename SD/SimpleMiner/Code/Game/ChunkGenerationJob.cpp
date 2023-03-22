#include "Game/ChunkGenerationJob.hpp"
#include "Game/Chunk.hpp"

ChunkGenerationJob::ChunkGenerationJob(Chunk* chunk)
	: Job(CHUNK_GEN_JOB_TYPE)
	, m_chunk(chunk)
{
}


void ChunkGenerationJob::Execute()
{
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATING;
	m_chunk->GenerateBlocks();
	m_chunk->m_state = ChunkState::ACTIVATING_GENERATE_DONE;
}


void ChunkGenerationJob::OnFinished()
{

}


