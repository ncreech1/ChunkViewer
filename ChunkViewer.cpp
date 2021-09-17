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
	return 0;

    return -1;
}
