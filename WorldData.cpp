#include "WorldData.h"

using namespace std;

void WorldData::addChunk(Chunk* chunk)
{
    string chunkID;

    chunkID = "" + chunk->x + chunk->z;

    chunks[chunkID] = chunk;
}

Chunk* WorldData::findChunk(string id)
{
    unordered_map<string, Chunk*>::iterator it;
    
    it = chunks.find(id);

    if(it != chunks.end())
	return it->second;

    return nullptr;
}

void WorldData::addNetherChunk(Chunk* chunk)
{
    string chunkID;

    chunkID = "" + chunk->x + chunk->z;

    netherChunks[chunkID] = chunk;
}

Chunk* WorldData::findNetherChunk(string id)
{
    unordered_map<string, Chunk*>::iterator it;
    
    it = netherChunks.find(id);

    if(it != netherChunks.end())
	return it->second;

    return nullptr;
}

void WorldData::addEndChunk(Chunk* chunk)
{
    string chunkID;

    chunkID = "" + chunk->x + chunk->z;

    endChunks[chunkID] = chunk;
}

Chunk* WorldData::findEndChunk(string id)
{
    unordered_map<string, Chunk*>::iterator it;
    
    it = endChunks.find(id);

    if(it != endChunks.end())
	return it->second;

    return nullptr;
}

void WorldData::freeMemory()
{
    unordered_map<string, Chunk*>::iterator it;

    for(it = chunks.begin(); it != chunks.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }

    for(it = netherChunks.begin(); it != netherChunks.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }

    for(it = endChunks.begin(); it != endChunks.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }
}
