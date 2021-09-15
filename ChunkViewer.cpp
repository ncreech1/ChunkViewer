#include <iostream>
#include <string>
#include "ParseBedrock.h"

using namespace std;

int main(int argc, char **argv)
{
	string dbpath;

	dbpath = argv[1];

	if(parseBedrock(dbpath))
		return 0;

	return -1;
}
