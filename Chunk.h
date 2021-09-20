#pragma once
#include <string>
#include <vector>
#include "NBTTag.h"

struct Block
{
    int x, y, z;
    int rawX, rawY, rawZ;
    int state;
    std::string name;
};

class Chunk
{
    public:
	int x, y, z;
	Chunk();
	void addBlock(Block* block);
	void addBlockState(int y, NBTTag* state);
	Block* getBlock(int x, int y, int z);
	NBTTag* getBlockState(int y, int state);
	void freeMemory();
    private:
	std::vector<std::vector<std::vector<Block*>>> blocks;
	std::vector<std::vector<NBTTag*>> blockStates;
};
