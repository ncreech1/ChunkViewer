#include "NBTTag.h"

using namespace std;

void NBTTag::addInnerTag(NBTTag *tag)
{
    innerTags[tag->name] = tag;
}
