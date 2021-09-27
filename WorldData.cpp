#include <iostream>
#include <cmath>
#include "WorldData.h"

using namespace std;

//Adds a chunk to the world
void WorldData::addChunk(Chunk *chunk, ChunkDimension dimension)
{
    string chunkID;

    chunkID = "" + to_string(chunk->x) + "|" + to_string(chunk->z);

    if(dimension == ChunkDimension::OVERWORLD)
	chunks[chunkID] = chunk;
    else if(dimension == ChunkDimension::NETHER)
	netherChunks[chunkID] = chunk;
    else
	endChunks[chunkID] = chunk;
}

//Returns a pointer to the chunk with id "chunkX|chunkZ"
//Returns a nulltpr on failure
Chunk* WorldData::findChunk(string id, ChunkDimension dimension)
{
    unordered_map<string, Chunk*>::iterator it;
    
    if(dimension == ChunkDimension::OVERWORLD)
    {
	it = chunks.find(id);

	if(it != chunks.end())
	    return it->second;
    }
    
    else if(dimension == ChunkDimension::NETHER)
    {
	it = netherChunks.find(id);
	
	if(it != netherChunks.end())
	    return it->second;
    }

    else
    {
	it = endChunks.find(id);

	if(it != endChunks.end())
	    return it->second;
    }

    return nullptr;
}

//Returns a block at (x, y, z)
//Returns a block with state < 0 on failure
Block WorldData::getBlock(int x, int y, int z, ChunkDimension dimension)
{
    int chunkX, chunkZ, rawX, rawZ;
    string chunkID;
    Chunk *blockChunk;
    Block result;

    chunkX = floor(x / 16.0);
    chunkZ = floor(z / 16.0);
    rawX = x - 16 * chunkX;
    rawZ = z - 16 * chunkZ;

    chunkID = "" + to_string(chunkX) + "|" + to_string(chunkZ);
    blockChunk = findChunk(chunkID, dimension);

    if(blockChunk == nullptr)
	result.state = -1;
    else
	result = blockChunk->getBlock(rawX, y, rawZ);

    return result;
}

//Returns the chunk a block at (x, y, z) resides in
//Returns nullptr on failure
Chunk* WorldData::getBlockChunk(int x, int y, int z, ChunkDimension dimension)
{
    int chunkX, chunkZ, rawX, rawZ;
    string chunkID;
    Chunk *result;

    chunkX = floor(x / 16.0);
    chunkZ = floor(z / 16.0);
    rawX = x - 16 * chunkX;
    rawZ = z - 16 * chunkZ;

    chunkID = "" + to_string(chunkX) + "|" + to_string(chunkZ);
    result = findChunk(chunkID, dimension);

    return result;
}

//Frees all memory associated with this world
void WorldData::freeMemory()
{
    unordered_map<string, Chunk*>::iterator it;
    
    //Free overworld chunks
    for(it = chunks.begin(); it != chunks.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }

    //Free nether chunks
    for(it = netherChunks.begin(); it != netherChunks.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }

    //Free end chunks
    for(it = endChunks.begin(); it != endChunks.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }
}
