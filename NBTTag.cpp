#include "NBTTag.h"

using namespace std;

void NBTTag::addInnerTag(NBTTag *tag)
{
    innerTags[tag->name] = tag;
}

NBTTag* NBTTag::findInnerTag(string key)
{
    unordered_map<string, NBTTag*>::iterator it;

    it = innerTags.find(key);

    if(it != innerTags.end())
	return it->second;

    return nullptr;
}

void NBTTag::freeMemory()
{
    unordered_map<string, NBTTag*>::iterator it;

    for(it = innerTags.begin(); it != innerTags.end(); it++)
    {
	it->second->freeMemory();
	delete it->second;
    }
}

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
