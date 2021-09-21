#pragma once
#include <string>
#include <vector>
#include "NBTTag.h"

struct Block
{
    int state;
    NBTTag* stateTag;
};

class Chunk
{
    public:
	int x, y, z;
	Chunk();
	void addBlock(int state, int rawX, int y, int rawZ);
	void addBlockState(int y, NBTTag* state);
	Block getBlock(int rawX, int y, int rawZ);
	NBTTag* getBlockState(int y, int state);
	void freeMemory();
    private:
	std::vector<std::vector<std::vector<int>>> blocks;
	std::vector<std::vector<NBTTag*>> blockStates;
};
