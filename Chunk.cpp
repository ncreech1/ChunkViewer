#include "Chunk.h"

Chunk::Chunk()
{
    blocks.resize(16);

    for(int x = 0; x < 16; x++)
    {
	blocks[x].resize(256);

	for(int y = 0; y < 256; y++)
	    blocks[x][y].resize(16);
    }

    blockStates.resize(16);
}

void Chunk::addBlock(Block* block)
{
    blocks[block->x][block->y][block->z] = block;
}

Block* Chunk::getBlock(int x, int y, int z)
{
    return blocks[x][y][z];
}

void Chunk::addBlockState(int y, NBTTag *state)
{
    blockStates[y].push_back(state);
}

NBTTag* Chunk::getBlockState(int y, int state)
{
    return blockStates[y][state];
}

void Chunk::freeMemory()
{
    for(int x = 0; x < 16; x++)
	for(int y = 0; y < 256; y++)
	    for(int z = 0; z < 16; z++)
		delete blocks[x][y][z];

    for(int y = 0; y < 16; y++)
    {
	for(int x = 0; x < blockStates[y].size(); x++)
	{
	    blockStates[y][x]->freeMemory();
	    delete blockStates[y][x];
	}
    }
}
