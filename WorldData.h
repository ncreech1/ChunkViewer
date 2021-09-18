#pragma once
#include <unordered_map>
#include <string>
#include "Chunk.h"

class WorldData
{
    public:
	void addChunk(Chunk* chunk);
	void addNetherChunk(Chunk* chunk);
	void addEndChunk(Chunk* chunk);
	Chunk* findChunk(std::string id);
	Chunk* findNetherChunk(std::string id);
	Chunk* findEndChunk(std::string id);
	void freeMemory();
    private:
	std::unordered_map<std::string, Chunk*> chunks;
	std::unordered_map<std::string, Chunk*> netherChunks;
	std::unordered_map<std::string, Chunk*> endChunks;
};
