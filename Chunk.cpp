#include "Chunk.h"

//Constructor initializes the 3D block vector and 2D block state vector
Chunk::Chunk()
{
    blocks.resize(16);

    for(int x = 0; x < 16; x++)
    {
	blocks[x].resize(16);
	for(int z = 0; z < 16; z++)
	    blocks[x][z].resize(256, -1);
    }

    blockStates.resize(16);
}

//Adds a block to the 3D block vector
void Chunk::addBlock(int state, int rawX, int y, int rawZ)
{
    blocks[rawX][rawZ][y] = state;
}

//Returns a block from the 3D block vector
//Returns a block with state = -1 on failure
Block Chunk::getBlock(int rawX, int y, int rawZ)
{
    Block result;
    NBTTag* blockState;

    result.state = blocks[rawX][rawZ][y];

    if(result.state != -1)
	result.stateTag = blockStates[y / 16][result.state];
    
    return result;
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
