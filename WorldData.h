#pragma once
#include <unordered_map>
#include <string>
#include "Chunk.h"

enum ChunkDimension
{
    OVERWORLD,
    NETHER,
    ENDWORLD
};

class WorldData
{
    public:
	Block getBlock(int x, int y, int z, ChunkDimension dimension);
	Chunk* getBlockChunk(int x, int y, int z, ChunkDimension dimension);
	void addChunk(Chunk* chunk, ChunkDimension dimension);
	Chunk* findChunk(std::string id, ChunkDimension dimension);
	void freeMemory();
    private:
	std::unordered_map<std::string, Chunk*> chunks;
	std::unordered_map<std::string, Chunk*> netherChunks;
	std::unordered_map<std::string, Chunk*> endChunks;
};
