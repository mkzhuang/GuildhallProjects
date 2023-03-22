#pragma once
#include "Engine/Core/JobSystem.hpp"

class Chunk;

constexpr uint8_t CHUNK_GEN_JOB_TYPE = 0b00000001;

class ChunkGenerationJob : public Job
{
public:
	ChunkGenerationJob(Chunk* chunk);
	~ChunkGenerationJob() {}

public:
	virtual void Execute() override;
	virtual void OnFinished() override;

public:
	Chunk* m_chunk = nullptr;
};