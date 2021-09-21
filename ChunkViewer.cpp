#include <iostream>
#include <string>
#include "ParseBedrock.h"
#include "WorldData.h"
#include "Chunk.h"

using namespace std;

int main(int argc, char **argv)
{
    WorldData world;
    string dbpath;

    dbpath = argv[1];

    if(parseBedrock(dbpath, world))
    {
	/*Block test;
	test = world.getBlock(672, 73, 22, ChunkDimension::OVERWORLD);
	
	if(test.state != -1)
	{
	    cerr << test.stateTag->findInnerTag("name")->stringVal << endl;
	}*/

	world.freeMemory();
    }

    return -1;
}
