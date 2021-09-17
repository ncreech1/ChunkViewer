#pragma once
#include <string>
#include <vector>
#include "NBTTag.h"

struct Block
{
    int x, y, z;
    int state;
    std::string name;
};

class Chunk
{
    public:
	Chunk();
	void addBlock(Block* block);
	void addBlockState(NBTTag* state);
    private:
	std::vector<std::vector<std::vector<Block*>>> blocks;
	std::vector<NBTTag*> blockStates;
};
