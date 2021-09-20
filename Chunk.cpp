#include "Chunk.h"

//Constructor initializes the 3D block vector and 2D block state vector
Chunk::Chunk()
{
    blocks.resize(16);

    for(int x = 0; x < 16; x++)
	blocks[x].resize(16);

    blockStates.resize(16);
}

//Adds a block to the 3D block vector
void Chunk::addBlock(Block *block)
{
    blocks[block->rawX][block->rawZ].push_back(block);
}

//Returns a block from the 3D block vector
Block* Chunk::getBlock(int x, int y, int z)
{
    return blocks[x][z][y];
}

//Adds a block state NBTTag to the block state vector for subchunk y
void Chunk::addBlockState(int y, NBTTag *state)
{
    blockStates[y].push_back(state);
}

//Returns a block state NBTTag from the block state vector
NBTTag* Chunk::getBlockState(int y, int state)
{
    return blockStates[y][state];
}

//Frees all memory associated with this chunk
void Chunk::freeMemory()
{
    //Delete all 4096 block pointers
    /*for(int x = 0; x < 16; x++)
	for(int y = 0; y < 256; y++)
	    for(int z = 0; z < 16; z++)
		delete blocks[x][y][z];*/

    for(int x = 0; x < 16; x++)
	for(int z = 0; z < 16; z++)
	    for(int y = 0; y < blocks[x][z].size(); y++)
		delete blocks[x][z][y];

    //Delete all block states
    for(int y = 0; y < 16; y++)
    {
	for(int x = 0; x < blockStates[y].size(); x++)
	{
	    blockStates[y][x]->freeMemory();
	    delete blockStates[y][x];
	}
    }
}
