#include "NBTTag.h"

using namespace std;

//Adds a new inner tag to this tag
void NBTTag::addInnerTag(NBTTag *tag)
{
    innerTags[tag->name] = tag;
}

//Searches this tag's inner tags recursively for one with the name key
NBTTag* NBTTag::findInnerTag(string key)
{
    unordered_map<string, NBTTag*>::iterator it;
    NBTTag *t;

    //Try surface level tags first
    it = innerTags.find(key);
    if(it != innerTags.end())
	return it->second;

    //Recursively search inner tags
    for(it = innerTags.begin(); it != innerTags.end(); it++)
    {
	t = it->second->findInnerTag(key);

	if(t != nullptr)
	    return t;
    }

    return nullptr;
}

//Recursively frees all memory associated with this tag
void NBTTag::freeMemory()
{
    unordered_map<string, NBTTag*>::iterator it;

    for(it = innerTags.begin(); it != innerTags.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }
}

//Returns the specific name for a block (if there is one)
string NBTTag::getBlockType()
{
    NBTTag *t;
    string containsType;
    string blockName;

    blockName = findInnerTag("name")->stringVal;

    if(blockName == "minecraft:stone")
    {
	t = findInnerTag("stone_type");

	if(t->stringVal == "stone")
	    return "stone";
	else
	    return "stone_" + t->stringVal;
    }

    if(blockName == "minecraft:leaves" || blockName == "minecraft:leaves2")
    {
	t = findInnerTag("old_leaf_type");

	if(t != nullptr)
	    return "leaves_" + t->stringVal + "_opaque";
	else
	{
	    t = findInnerTag("new_leaf_type");
	    return "leaves_" + t->stringVal + "_opaque";
	}
    }

    if(blockName == "minecraft:log")
    {
	t = findInnerTag("old_log_type");

	if(t != nullptr)
	    return "log_" + t->stringVal;
	else
	{
	    t = findInnerTag("new_log_type");
	    return "log" + t->stringVal;
	}
    }

    if(blockName == "minecraft:sand")
    {
	t = findInnerTag("sand_type");

	if(t->stringVal == "normal")
	    return "sand";
	else
	    return t->stringVal;
    }

    if(blockName == "minecraft:grass")
	return "grass_side_carried";

    if(blockName == "minecraft:sandstone")
	return "sandstone_normal";
    if(blockName == "minecraft:water")
	return "water_still";
    if(blockName == "minecraft:lava")
	return "lava_still";

    return "NULL";
}

//For use in debugging; prints all the inner tags' names recursively
string NBTTag::printInnerTags()
{
    unordered_map<string, NBTTag*>::iterator it;
    string toPrint;

    toPrint = "";
    for(it = innerTags.begin(); it != innerTags.end(); it++)
    {
	if(it->second->type == NBTType::STRING)
	    toPrint += it->second->name + ":" + it->second->stringVal + " ";
	toPrint += it->second->printInnerTags();
    }

    return toPrint;
}
