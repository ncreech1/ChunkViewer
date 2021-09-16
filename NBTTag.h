#pragma once
#include <string>

enum NBTType
{
    END,
    BYTE,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    BYTE_ARRAY,
    STRING,
    LIST,
    COMPOUND,
    INT_ARRAY,
    LONG_ARRAY
};

class NBTTag
{
    public:
	NBTType type;
	uint16_t nameLength;
	std::string name;
	char *payload;
};
