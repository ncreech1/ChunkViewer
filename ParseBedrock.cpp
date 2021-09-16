#include <iostream>
#include <iomanip>
#include <cmath>
#include "ParseBedrock.h"
#include "NBTTag.h"
#include "leveldb/db.h" 
//#include "leveldb/env.h"
//#include "leveldb/filter_policy.h"
//#include "leveldb/cache.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/decompress_allocator.h"

using namespace std;

//Parses a little endian uint32 from a Slice
uint32_t parseInt32(leveldb::Slice &s, uint32_t offset)
{
    uint32_t result;

    result = 0;
    for(int i = 0; i < 4; i++)
    {
	//Static cast prevents sign extension
	result |= (static_cast<uint8_t>(s[offset + i] << i * 8));
    }

    return result;
}

//Parses a little endian uint16 from a Slice
uint16_t parseInt16(leveldb::Slice &s, uint16_t offset)
{
    uint16_t result;

    result = 0;
    for(int i = 0; i < 2; i++)
    {
	//Static cast prevents sign extension
	result |= (static_cast<uint8_t>(s[offset + i] << i * 8));
    }

    return result;
}

//Parses a little endian varint from a Slice
uint32_t parseVarint(leveldb::Slice &s, uint32_t offset)
{
    uint32_t result;
    int i;

    i = 0;
    result = 0;
    
    while(1)
    {
	unsigned char piece;

	piece = s[offset + i];
	piece &= 0x7F; //Mask out the MSB
	result |= (piece << i * 7);

	//Zero in MSB indicates end of varint
	if(s[offset + i] & 0x80 == 0)
	    break;
	
	i++;
    }

    return result;
}

//Parses a Slice for an NBTTag payload and returns the new offset 
int parseNBTPayload(leveldb::Slice &s, uint32_t offset, NBTTag &tag)
{
    char byte;

    switch(tag.type)
    {
	case NBTType::END:
	    break;
	case NBTType::BYTE:
	    byte = s[offset];
	    tag.payload = &byte;
	    offset++;
	    break;
	case NBTType::SHORT:
	    //tag.payload = 0 | s[offset] | (s[offset + 1] << 8);
	    //offset += 2;
	    break;
	case NBTType::INT:
	    //byte = parseVarint(s, offset));
	    //tag.payload = &byte;
	    //offset += 4;
	    break;
	case NBTType::LONG:
	    break;
	case NBTType::FLOAT:
	    break;
	case NBTType::DOUBLE:
	    break;
	case NBTType::BYTE_ARRAY:
	    break;
	case NBTType::STRING:
	    break;
	case NBTType::LIST:
	    break;
	case NBTType::COMPOUND:
	    break;
	case NBTType::INT_ARRAY:
	    break;
	case NBTType::LONG_ARRAY:
	    break;
    }

    return offset;
}

//Parses a Slice for an NBTTag and returns the new offset
int parseNBTTag(leveldb::Slice &s, uint32_t offset, NBTTag &tag)
{
    uint16_t nameLength;

    //Parse the data type of the tag
    tag.type = static_cast<NBTType>(s[offset]);
    offset++;

    //Parse the length of the tag name
    nameLength = parseInt16(s, offset);
    offset += 2;

    //Parse the tag name
    for(int i = 0; i < nameLength; i++)
	tag.name += static_cast<char>(s[offset + i]);
    offset += nameLength;

    //Parse the tag payload
    offset = parseNBTPayload(s, offset, tag);

    return offset;
}

//Parses a Minecraft Bedrock db folder for chunk data 
bool parseBedrock(string path)
{
    leveldb::DB *db;
    leveldb::Options options;
    leveldb::ReadOptions readOptions;
    leveldb::Status status;
    leveldb::Iterator *lit;

    //Use the new raw-zip compressor to write (and read)
    options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);

    //Also setup the old, slower compressor for backwards compatibility. 
    //This will only be used to read old compressed blocks.
    options.compressors[1] = new leveldb::ZlibCompressor();

    //Create a reusable memory space for decompression so it allocates less
    readOptions.decompress_allocator = new leveldb::DecompressAllocator();

    status = leveldb::DB::Open(options, path, &db);

    //Error opening db folder
    if(!status.ok())
    {
	cout << "Could not open db folder '" << path << "'" << endl;
	return false;
    }

    lit = db->NewIterator(readOptions);

    //Iterate the db table
    int totalChunks = 0;
    for(lit->SeekToFirst(); lit->Valid(); lit->Next())
    {
	leveldb::Slice k, v;
		
	k = lit->key();
	v = lit->value();

	//Keys for chunks are between 9 and 14 bytes
	if(k.size() >= 9 && k.size() <= 14)
	{
	    uint32_t x, y, z;
	    uint32_t regionID;
	    string regionName;

	    x = parseInt32(k, 0);
	    z = parseInt32(k, 4);
	    regionName = "Overworld";

	    if(k.size() == 13 || k.size() == 14)
	    {
		regionID = parseInt32(k, 8);
		regionName = (regionID == 1 ? "Nether" : "End");
	    }
	
	    //Keys of size 10 and 14 with the second to last byte equal to '/' are
	    //sub chunk prefix keys describing 16x16x16 blocks of terrain		
	    if((k.size() == 10 && k[8] == 0x2F) || (k.size() == 14 && k[12] == 0x2F))
	    {	
		int currOffset;
		int chunkFormat, storageNum;

		y = k[k.size() - 1];
		currOffset = 0;

		cerr << "Found " << regionName << " Chunk at ";
		cerr << "(" << x << ", " << y << ", " << z << "): ";
		
		//The type of format this sub chunk uses
		chunkFormat = v[currOffset];
		currOffset++;

		//Sub chunk format should always be 0x08
		if(chunkFormat != 8)
		{
		    cerr << "Old sub chunk format! Skipping..." << endl;
		    continue;
		}

		//The number of block storages
		storageNum = v[currOffset];
		cerr << storageNum << " ";
		currOffset++;

		//Chunk format is followed by groups of bytes called block storages
		for(int i = 0; i < storageNum; i++)
		{
		    int storageVersion;
		    int bitsPerBlock, blocksPerWord, numWords;
		    int blockDataOffset, palletteOffset;
		    int32_t paletteSize;

		    storageVersion = v[currOffset];
		    currOffset++;
		    
		    //Shouldn't happen since program is run on persistent storage
		    if(storageVersion & 0x01 == 1)
		    {
			cerr << "Error: Block serialized for runtime!" << endl;
			return false;
		    }

		    bitsPerBlock = storageVersion >> 1;
		    blocksPerWord = 32 / bitsPerBlock;

		    /*3, 5, and 6 blocks per word require padding bytes between 
		    //blocks and are not used in normal minecraft saves
		    if(32 % bitsPerBlock != 0)
		    {
			cerr << bitsPerBlock << " Error: Found padding compression!" << endl;
			return false;
		    }*/

		    numWords = ceil(4096 / blocksPerWord);
		    blockDataOffset = currOffset;
		    currOffset += numWords * 4;

		    paletteSize = parseVarint(v, currOffset);
		    currOffset += 4;

		    //Read palette data (block IDs and data)
		    for(int p = 0; p < paletteSize; p++)
		    {
			//NBTTag tag;
			//currOffset = parseNBTTag(v, currOffset, &tag);
		    }

		    //Read block state data and get positions
		    for(int w = 0; w < numWords; w++)
		    {
			//Data starts from blockDataOffset
		    }

		    cerr << "Number of Words: " << numWords << " PSize: " << paletteSize << endl;
		}

		cerr << endl;
	    	
		/*for(int i = 0; i < v.size(); i++)
		{
		    fprintf(stderr, "%02x ", (unsigned char)v[i]);
		}*/
	    }

	    totalChunks++;
	}
    }

    cerr << "Total Chunks: " << totalChunks << endl;

    return true;
}
