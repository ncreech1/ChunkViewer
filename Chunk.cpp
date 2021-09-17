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
}

void Chunk::addBlock(Block* block)
{
    blocks[block->x][block->y][block->z] = block;
}

void Chunk::addBlockState(NBTTag *state)
{
    blockStates.push_back(state);
}
