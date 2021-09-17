#include "WorldData.h"

using namespace std;

Chunk* WorldData::findChunk(string id)
{
    unordered_map<string, Chunk*>::iterator it;
    
    it = chunks.find(id);

    if(it != chunks.end())
	return it->second;

    return nullptr;
}
