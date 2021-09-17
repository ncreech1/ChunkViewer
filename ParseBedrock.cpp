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

int parseNBTTag(leveldb::Slice &s, uint32_t offset, NBTTag &tag, bool parseType = true);

//Parses a little endian int32 from a Slice
int32_t parseInt32(leveldb::Slice &s, int32_t offset)
{
    int32_t result;

    result = 0;
    for(int i = 0; i < 4; i++)
    {
	//Static cast prevents sign extension
	result |= (static_cast<uint8_t>(s[offset + i] << i * 8));
    }

    return result;
}

//Parses a little endian int16 from a Slice
int16_t parseShort(leveldb::Slice &s, int16_t offset)
{
    int16_t result;

    result = 0;
    for(int i = 0; i < 2; i++)
    {
	//Static cast prevents sign extension
	result |= (static_cast<uint8_t>(s[offset + i] << i * 8));
    }

    return result;
}

//Parses a little endian varint from a Slice and modifies the offset reference
int32_t parseVarint(leveldb::Slice &s, uint32_t &offset)
{
    int32_t result;
    uint8_t byte;
    int i;
    
    i = 0;
    result = 0;
    while(1)
    {
	byte = s[offset + i];

	//Mask out the MSB and shift
	result |= static_cast<uint8_t>(byte & 0x7F) << (i * 7);

	//Zero in MSB indicates end of varint
	//Static cast to prevent sign extension
	if(static_cast<uint8_t>(byte & 0x80u) == 0)
	    break;
	
	i++;
    }

    offset += i + 1;
    return result;
}

//Parses a little endian varlong from a Slice and modifies the offset reference
int64_t parseVarlong(leveldb::Slice &s, uint32_t &offset)
{
    int64_t result;
    uint8_t byte;
    int i;
    
    i = 0;
    result = 0;
    while(1)
    {
	byte = s[offset + i];

	//Mask out the MSB and shift
	result |= static_cast<uint8_t>(byte & 0x7F) << (i * 7);

	//Zero in MSB indicates end of varint
	//Static cast to prevent sign extension
	if(static_cast<int8_t>(byte & 0x80u) == 0)
	    break;
	
	i++;
    }

    offset += i + 1;
    return result;
}

//Returns the varint as the signed decoding of the zig zag
int32_t parseVarintZZ(leveldb::Slice s, uint32_t &offset)
{
    int32_t encoded;

    encoded = parseVarint(s, offset);
    
    return (encoded & 1) ? (encoded >> 1) ^ -1 : (encoded >> 1);
}

//Returns the varlong as the signed decoding of the zig zag
int64_t parseVarlongZZ(leveldb::Slice s, uint32_t &offset)
{
    int64_t encoded;

    encoded = parseVarlong(s, offset);
    
    return (encoded & 1) ? (encoded >> 1) ^ -1 : (encoded >> 1);
}

//Parses a Slice for an NBTTag payload and returns the new offset 
int parseNBTPayload(leveldb::Slice &s, uint32_t offset, NBTTag &tag)
{
    uint32_t stringLength;
    uint32_t listLength;
    NBTType listType;

    switch(tag.type)
    {
	case NBTType::BYTE:
	    tag.byteVal = s[offset];
	    offset++;
	    //cerr << "Byte Payload: " << tag.byteVal << endl;
	    break;
	case NBTType::SHORT:
	    tag.shortVal = parseShort(s, offset);
	    offset += 2;
	    break;
	case NBTType::INT:
	    //cerr << "Parsing Int Tag " << tag.name << endl;
	    tag.intVal = parseInt32(s, offset);
	    offset += 4;
	    //cerr << "Int Payload: " << tag.intVal << endl;
	    //cerr << "End of Int Tag" << endl;
	    break;
	case NBTType::LONG:
	    tag.longVal = parseVarlongZZ(s, offset);
	    break;
	case NBTType::STRING:
	    //cerr << "Parsing String Tag " << tag.name << endl;
	    stringLength = parseShort(s, offset);
	    offset += 2;
	    tag.stringVal = "";
	    for(int x = 0; x < stringLength; x++)
	    {
		tag.stringVal += static_cast<char>(s[offset]);
		offset++;
	    }
	    //cerr << "String Payload: " << tag.stringVal << endl;
	    //cerr << "End of String Tag" << endl;
	    break;
	case NBTType::LIST:
	    listType = static_cast<NBTType>(s[offset]);
	    offset++;
	    listLength = parseVarintZZ(s, offset);
	    for(int x = 0; x < listLength; x++)
	    {
		NBTTag innerTag;
		innerTag.type = listType;
		offset = parseNBTTag(s, offset, innerTag, false);
		tag.addInnerTag(innerTag);
	    }
	    break;
	case NBTType::COMPOUND:
	    //cerr << "Parsing Compound Tag " << tag.name << endl;
	    while(s[offset] != 0)
	    {
		NBTTag innerTag;
		offset = parseNBTTag(s, offset, innerTag);
		tag.addInnerTag(innerTag);
	    }
	    //cerr << "End of Compound Tag " << tag.name << endl;
	    offset++;
	    break;
    }

    return offset;
}

//Parses a Slice for an NBTTag and returns the new offset
int parseNBTTag(leveldb::Slice &s, uint32_t offset, NBTTag &tag, bool parseType /*=true*/)
{
    int32_t nameLength;

    //Parse the data type of the tag
    if(parseType)
    {
	tag.type = static_cast<NBTType>(s[offset]);
	offset++;
    }

    //cerr << "Found tag of type " << tag.type << endl;

    //Parse the length of the tag name
    nameLength = parseShort(s, offset);
    offset += 2;
    tag.name = "";

    //cerr << "Name length: " << nameLength << endl;

    //Parse the tag name
    for(int i = 0; i < nameLength; i++)
    {
	tag.name += static_cast<unsigned char>(s[offset]);
	offset++;
    }

    //cerr << "Name: " << tag.name << endl;

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
	    int32_t chunkX, chunkY, chunkZ;
	    int32_t regionID;
	    string regionName;

	    chunkX = parseInt32(k, 0);
	    chunkZ = parseInt32(k, 4);
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
		uint32_t currOffset;
		int chunkFormat, storageNum;

		chunkY = k[k.size() - 1];
		currOffset = 0;

		//cerr << "Found " << regionName << " Chunk at ";
		//cerr << "(" << chunkX << ", " << chunkY << ", " << chunkZ << ")" << endl;

		//for(int x = 0; x < v.size(); x++)
		  //  fprintf(stderr, "%02x ", (unsigned char)v[x]);

		cerr << dec;
		
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
		//cerr << "Block Storages: " << storageNum << endl;
		currOffset++;

		//Chunk format is followed by groups of bytes called block storages
		for(int i = 0; i < storageNum; i++)
		{
		    int storageVersion;
		    int bitsPerBlock, blocksPerWord, numWords;
		    int blockDataOffset, paletteOffset;
		    int32_t paletteSize;
		    int position;

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

		    numWords = ceil(4096.0 / blocksPerWord);
		    blockDataOffset = currOffset;
		    currOffset += numWords * 4;

		    paletteSize = parseInt32(v, currOffset);
		    currOffset += 4;

		    //cerr << "Storage " << i << ": BPB: " << bitsPerBlock << " BPW: " << blocksPerWord << " Words: " << numWords << " PSize: " << paletteSize << endl;

		    //for(int x = currOffset; x < v.size(); x++)
			//fprintf(stderr, "%02x ", static_cast<uint8_t>(v[x]));

		    //Read palette data (block IDs and data)
		    for(int p = 0; p < paletteSize; p++)
		    {
			NBTTag tag;
			currOffset = parseNBTTag(v, currOffset, tag);
		    }

		    paletteOffset = currOffset;
		    currOffset = blockDataOffset;
		    position = 0;

		    //Read block state data and get positions
		    for(int w = 0; w < numWords; w++)
		    {
			int32_t word = parseInt32(v, currOffset);
			currOffset += 4;

			for(int block = 0; block < blocksPerWord; block++)
			{
			    int state = (word >> ((position % blocksPerWord) * bitsPerBlock)) & ((1 << bitsPerBlock) - 1);
			    int x = (position >> 8) & 0xF;
			    int y = position & 0xF;
			    int z = (position >> 4) & 0xF;

			    //cerr << "Found block " << state << " at (" << x << ", " << y << ", " << z << ")" << endl;
			    position++;
			}
		    }

		    currOffset = paletteOffset;

		    //cerr << "Number of Words: " << numWords << " PSize: " << paletteSize << endl;
		}

		//cerr << endl;
	    }

	    totalChunks++;
	}
    }

    cerr << "Total Sub Chunks: " << totalChunks << endl;

    return true;
}
