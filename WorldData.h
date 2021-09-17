#pragma once
#include <unordered_map>
#include <string>
#include "Chunk.h"

class WorldData
{
    public:
	Chunk* findChunk(std::string id);
    private:
	std::unordered_map<std::string, Chunk*> chunks;
};
