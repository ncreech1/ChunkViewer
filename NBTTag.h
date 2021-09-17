#pragma once
#include <string>
#include <map>

enum NBTType
{
    END = 0,
    BYTE = 1,
    SHORT = 2,
    INT = 3,
    LONG = 4,
    STRING = 8,
    LIST = 9,
    COMPOUND = 10
};

class NBTTag
{
    public:
	NBTType type;
	uint16_t nameLength;
	std::string name;
	uint32_t intVal;
	uint16_t shortVal;
	uint8_t byteVal;
	uint64_t longVal;
	std::string stringVal;
	void addInnerTag(NBTTag tag);
    private:
	std::map<std::string, NBTTag> innerTags;
};
