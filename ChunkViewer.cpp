#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include "ParseBedrock.h"
#include "WorldData.h"
#include "Chunk.h"

using namespace std;

int* parseCoords(string word)
{
    string xString, yString, zString;
    int currentPos;
    bool valid;
    int *xyz;

    valid = true;
    xyz = new int[3];

    if(word[0] != '(' || word[word.length() - 1] != ')')
    {
	cout << "Coordinates must be in format (x,y,z)" << endl;
	delete[] xyz;
	return nullptr;
    }

    currentPos = 0;
    for(int i = 1; i < word.length() - 1; i++)
    {			
	if(word[i] == ',')
	{
	    currentPos++;
	    continue;
	}

	if(currentPos == 0)
	    xString += word[i];
	else if(currentPos == 1)
	    yString += word[i];
	else if(currentPos == 2)
	    zString += word[i];
	else { valid = false; break; }
    }

    try
    {
	xyz[0] = stoi(xString);
	xyz[1] = stoi(yString);
	xyz[2] = stoi(zString);
    }

    catch(exception &e) { valid = false; }

    if(!valid || currentPos != 2)
    {
	cout << "Coordinates must be in format (x,y,z)" << endl;
	delete[] xyz;
	return nullptr;
    }   

    return xyz;
}

bool parseInt(const string &word, int &out)
{
    try { out = stoi(word); }
    catch(exception &e) 
    { 
	cout << "Error on parameter '" << word << "': Integer expected." << endl;
	return false;
    }

    return true;
}

void drawSlice(ofstream &file, Chunk *chunk, int *xyz, string &facing, int yMin, int yMax)
{
    Block currBlock;
    int inc, start;
    int rawX, rawZ;
    string name, path, blockType;
    struct stat buff;
    int index;

    rawX = xyz[0] - 16 * chunk->x;
    rawZ = xyz[2] - 16 * chunk->z;

    //Flip the direction blocks are printed depending on facing direction
    if(facing == "n" || facing == "e") { inc = 1; start = 0; }
    else { inc = -1; start = 15; }

    index = 0;
    for(int i = start; i >= 0 && i <= 15; i += inc)
    {
	//Travel along x or z coords
	if(facing == "n" || facing == "s")
	    rawX = i;
	else
	    rawZ = i;
	
	//Always print all y coords
	for(int y = yMin; y < yMax; y++)
	{
	    name = "";
	    currBlock = chunk->getBlock(rawX, y, rawZ);
				    
	    if(currBlock.state >= 0)
	    {
		name = currBlock.stateTag->findInnerTag("name")->stringVal;
		name = name.substr(10);
		path = "./blocks-eps/" + name + ".eps";
		blockType = currBlock.stateTag->getBlockType();

		//Air is not printed on graph
		if(name != "air" || blockType != "NULL")
		{
		    file << "newcurve eps ";
		    
		    //Try base name for texture
		    if(blockType == "NULL" && stat(path.c_str(), &buff) == 0)
			file << path << " ";
		    else
		    {
			//Try alternative name for texture
			path = "./blocks-eps/" + blockType + ".eps";

			if(stat(path.c_str(), &buff) == 0)
			    file << path << " ";
			else
			    file << "./blocks-eps/unknown.eps ";
		    }
		}
	    }
				
	    //Data corrupt block
	    else if(currBlock.state == -2)
		file << "newcurve eps ./blocks-eps/barrier.eps ";
	    //Unknown block
	    else
		file << "newcurve eps ./blocks-eps/unknown.eps ";
    
	    //Print the position on the graph
	    if(name != "air")
	    {
		file << "marksize 1 1 pts ";
		file << (0.52 + index) << " " << (0.52 + y) << endl; 
	    }
	}

	index++;
    }
}

int main(int argc, char **argv)
{
    WorldData world;
    string dbpath;
    string line;
    string word;

    dbpath = argv[1];

    //Read world data from db folder
    if(parseBedrock(dbpath, world))
    {	
	//Read in commands
	while(getline(cin, line, '\n'))
	{
	    stringstream ss(line);
	    ss >> word;

	    //Tell the user what is at a specific location
	    if(word == "what")
	    {
		bool valid = true;
		int currentPos = 0;
		string xString, yString, zString;
		int *xyz;
		Block b;
		ChunkDimension dim;
		
		ss >> word;

		//What block is at (x,y,z)?
		if(word == "block")
		{
		    //Default parameter values
		    dim = ChunkDimension::OVERWORLD;

		    //Get coordinates
		    ss >> word;
		    xyz = parseCoords(word);

		    //Invalid coordinate format
		    if(xyz == nullptr)
			continue;

		    //Read in optional flags
		    while(ss >> word)
		    {
			//Dimension flag
			if(word == "-d")
			{
			    ss >> word;
			    if(word == "overworld")
				dim = ChunkDimension::OVERWORLD;
			    else if(word == "nether")
				dim = ChunkDimension::NETHER;
			    else if(word == "end")
				dim = ChunkDimension::ENDWORLD;
			    else 
			    { 
				cout << "Dimension must be overworld, nether, or end." << endl;
				continue;
			    }
			}
		    }

		    //Retrieve block
		    b = world.getBlock(xyz[0], xyz[1], xyz[2], dim);

		    if(b.state < 0)
			cout << "Block not valid for this world." << endl;
		    else
		    {
			cout << "Block at (" << xyz[0] << "," << xyz[1] << "," << xyz[2] << ") is ";
			cout << b.stateTag->printInnerTags() << endl;
		    }

		    delete[] xyz;
		}

		else
		    cout << "Invalid 'what' argument '" << word << "'" << endl;
	    }

	    else if(word == "graph")
	    {
		Chunk *chunk;
		ChunkDimension dim;
		int *xyz;
		string facing;
		int yMin, yMax;
		bool foundFlags;
		string fileName;
		ofstream file;

		ss >> word;

		//Graph the slice a block is in from the given direction
		if(word == "block")
		{
		    //Default Parameters
		    dim = ChunkDimension::OVERWORLD;
		    yMin = 0;
		    yMax = 64;
		    facing = "n";
		    foundFlags = true;

		    //Get coordinates
		    ss >> word;
		    xyz = parseCoords(word);

		    //Invalid coordinate format
		    if(xyz == nullptr)
			continue;

		    //Get file name
		    ss >> fileName;
    		    bool validName = false;
		    for(int i = 0; i < fileName.size(); i++)
		    {
			if(fileName[i] != ' ')
			{
			    validName = true;
			    break;
			}
		    }

		    if(!validName)
		    {
			cout << "'" << fileName << "' is not a valid file name." << endl;
			continue;
		    }

		    //Read in optional flags
		    while(ss >> word)
		    {
			//Dimension flag
			if(word == "-d")
			{
			    ss >> word;
			    if(word == "overworld")
				dim = ChunkDimension::OVERWORLD;
			    else if(word == "nether")
				dim = ChunkDimension::NETHER;
			    else if(word == "end")
				dim = ChunkDimension::ENDWORLD;
			    else 
			    { 
				cout << "Dimension must be overworld, nether, or end." << endl;
				foundFlags = false;
				break;
			    }
			}

			//Facing flag
			else if(word == "-f")
			{
			    ss >> word;
			    facing = word;
			    if(word != "n" && word != "s" && word != "e" && word != "w")
			    {
				cout << "Direction must be n, s, e, or w." << endl;
				foundFlags = false;
				break;
			    }
			}
			
			//Graph y minimum flag
			else if(word == "-ymin")
			{
			    ss >> word;
			    if(!parseInt(word, yMin))
			    {
				foundFlags = false;
				break;
			    }
			}

			//Graph y maximum flag
			else if(word == "-ymax")
			{
			    ss >> word;
			    if(!parseInt(word, yMax))
			    {
				foundFlags = false;
				break;
			    }

			    yMax++; //Blocks should sit one below graph max
			}

			else
			{
			    cout << "'" << word << "' is not a valid flag." << endl;
			    foundFlags = false;
			    break;
			}
		    }

		    //An error occured while reading optional flags
		    if(!foundFlags)
			continue;
    
		    //Check graph bounds
		    if(yMax < 0 || yMax <= yMin || yMin < 0)
		    {
			cout << "Graph bounds must be positive and (y > 0)" << endl;
			continue;
		    }

		    //Retrieve the block
		    chunk = world.getBlockChunk(xyz[0], xyz[1], xyz[2], dim);

		    if(chunk == nullptr)
		    {
			cout << "Block not valid for this world." << endl;
			continue;
		    }

		    //Graph chunk slice using Jgraph
		    remove(fileName.c_str());
		    file.open(fileName.c_str());

		    if(!file.is_open())
		    {
			string err = "Error creating file '" + fileName + "':";
			perror(err.c_str());
			continue;
		    }
		    
		    string axis = (facing == "n" || facing == "s" ? "X" : "Z");
		    file << "newgraph" << endl;
		    file << "x_translate -7" << endl;
		    file << "y_translate " << ((64 / (yMax - yMin) - 1) * -1) << endl;
		    file << "xaxis min 0 max 16 size " << 2.5;
		    file << " hash 4 mhash 3 label : Block " << axis << endl;
		    file << "yaxis min " << yMin << " max " << yMax;
		    file << " size " << (2.5 * ((yMax - yMin) / 16));
		    file << " hash 4 mhash 3 label : Block Y" << endl;

		    //Draw chunk slice
		    drawSlice(file, chunk, xyz, facing, yMin, yMax);

		    file.close();
		    delete[] xyz;
		}

		else
		    cout << "Invalid 'graph' argument '" << word << "'" << endl;
	    }

	    else
		cout << "Unknown command '" << word << "'" << endl;
	}

	world.freeMemory();
    }

    return -1;
}
