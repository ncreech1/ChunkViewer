#include <iostream>
#include <iomanip>
#include <cmath>
#include "ParseBedrock.h"
#include "NBTTag.h"
#include "WorldData.h"
#include "Chunk.h"
#include "leveldb/db.h" 
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/decompress_allocator.h"

using namespace std;

int parseNBTTag(leveldb::Slice &s, uint32_t offset, NBTTag *tag, bool parseType = true);

//Parses a little endian int32 from a Slice
int32_t parseInt32(leveldb::Slice &s, int32_t offset)
{
    int32_t result;

    result = 0;
    for(int i = 0; i < 4; i++)
    {
	//Static cast prevents sign extension
	result |= (static_cast<uint8_t>(s[offset + i]) << i * 8);
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
	result |= (static_cast<uint8_t>(s[offset + i]) << i * 8);
    }

    return result;
}

//Parses a little endian int64 from a Slice
int64_t parseLong(leveldb::Slice &s, int16_t offset)
{
    int64_t result;

    result = 0;
    for(int i = 0; i < 8; i++)
    {
	//Static cast prevents sign extension
	result |= (static_cast<uint8_t>(s[offset + i]) << i * 8);
    }

    return result;
}
    
//Parses a Slice for an NBTTag payload and returns the new offset 
int parseNBTPayload(leveldb::Slice &s, uint32_t offset, NBTTag *tag)
{
    uint32_t stringLength;
    uint32_t listLength;
    NBTType listType;

    switch(tag->type)
    {
	case NBTType::BYTE:
	    tag->byteVal = s[offset];
	    offset++;
	    break;
	case NBTType::SHORT:
	    tag->shortVal = parseShort(s, offset);
	    offset += 2;
	    break;
	case NBTType::INT:
	    tag->intVal = parseInt32(s, offset);
	    offset += 4;
	    break;
	case NBTType::LONG:
	    tag->longVal = parseLong(s, offset);
	    offset += 8;
	    break;
	case NBTType::STRING:
	    stringLength = parseShort(s, offset);
	    offset += 2;
	    tag->stringVal = "";
	    for(int x = 0; x < stringLength; x++)
	    {
		tag->stringVal += static_cast<char>(s[offset]);
		offset++;
	    }
	    break;
	case NBTType::LIST:
	    listType = static_cast<NBTType>(s[offset]);
	    offset++;
	    listLength = parseInt32(s, offset);
	    offset += 4;
	    for(int x = 0; x < listLength; x++)
	    {
		NBTTag *innerTag = new NBTTag;
		innerTag->type = listType;
		offset = parseNBTTag(s, offset, innerTag, false);
		tag->addInnerTag(innerTag);
	    }
	    break;
	case NBTType::COMPOUND:
	    while(s[offset] != 0)
	    {
		NBTTag *innerTag = new NBTTag;
		offset = parseNBTTag(s, offset, innerTag);
		tag->addInnerTag(innerTag);
	    }
	    offset++;
	    break;
    }

    return offset;
}

//Parses a Slice for an NBTTag and returns the new offset
int parseNBTTag(leveldb::Slice &s, uint32_t offset, NBTTag *tag, bool parseType /*=true*/)
{
    int32_t nameLength;

    //Parse the data type of the tag
    if(parseType)
    {
	tag->type = static_cast<NBTType>(s[offset]);
	offset++;
    }

    //Parse the length of the tag name
    nameLength = parseShort(s, offset);
    offset += 2;
    tag->name = "";

    //Parse the tag name
    for(int i = 0; i < nameLength; i++)
    {
	tag->name += static_cast<unsigned char>(s[offset]);
	offset++;
    }

    //Parse the tag payload
    offset = parseNBTPayload(s, offset, tag);

    return offset;
}

//Parses a Minecraft Bedrock db folder for chunk data 
bool parseBedrock(string path, WorldData &world)
{
    leveldb::DB *db;
    leveldb::Options options;
    leveldb::ReadOptions readOptions;
    leveldb::Status status;
    leveldb::Iterator *lit;

    class NullLogger : public leveldb::Logger
    {
	public:
	    void Logv(const char*, va_list) override {}
    };

    options.filter_policy = leveldb::NewBloomFilterPolicy(10);
    options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);
    options.write_buffer_size = 4 * 1024 * 1024;
    options.info_log = new NullLogger;

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
	//Keys of 10 and 14 bytes ending in '/' describe a 16x16x16 sub chunk of terrain
	if((k.size() == 10 && k[8] == 0x2F) || (k.size() == 14 && k[12] == 0x2F))
	{
	    Chunk *currChunk;
	    string chunkID;
	    int32_t chunkX, chunkY, chunkZ;
	    int32_t regionID;
	    ChunkDimension dimension;
	    uint32_t currOffset;
	    int chunkFormat, storageNum;

	    //Set up basic chunk data (position, dimension, ID)
	    chunkX = parseInt32(k, 0);
	    chunkY = k[k.size() - 1];
	    chunkZ = parseInt32(k, 4);
	    dimension = ChunkDimension::OVERWORLD;
	    chunkID = "" + to_string(chunkX) + "|" + to_string(chunkZ);
	    currOffset = 0;

	    //Extra bytes describing Nether and End regions
	    if(k.size() == 13 || k.size() == 14)
	    {
		regionID = parseInt32(k, 8);
		dimension = (regionID == 1 ? ChunkDimension::NETHER : ChunkDimension::ENDWORLD);
	    }
	    
	    //Check if chunk has already been visited and add to world if not
	    currChunk = world.findChunk(chunkID, dimension);
	    if(currChunk == nullptr)
	    {
	        totalChunks++;
	        currChunk = new Chunk;
	        currChunk->x = chunkX;
	        currChunk->z = chunkZ;
	        world.addChunk(currChunk, dimension);
	    }

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
	    currOffset++;

	    //Sub chunk format is followed by groups of bytes called block storages
	    for(int i = 0; i < storageNum; i++)
	    {
	        int storageVersion;
	        int bitsPerBlock, blocksPerWord, numWords;
	        int blockDataOffset, paletteOffset;
	        int32_t paletteSize;
	        int pos;

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

		//Read palette data (block names and state data)
		for(int p = 0; p < paletteSize; p++)
		{
		    NBTTag *rootTag = new NBTTag;
		    currOffset = parseNBTTag(v, currOffset, rootTag);
		    currChunk->addBlockState(chunkY, rootTag);
		}

		paletteOffset = currOffset;
		currOffset = blockDataOffset;
		pos = 0;
		    
		//Read block position data and get state
		for(int w = 0; w < numWords; w++)
		{
		    int32_t word = parseInt32(v, currOffset);
		    currOffset += 4;

		    //Blocks are stored in XZY order
		    for(int block = 0; block < blocksPerWord; block++)
		    {
			int state, rawX, rawY, rawZ;
		   
			//When i > 0 this block has an alternate
			if(i == 0)
			{
			    state = (word >> ((pos % blocksPerWord) * bitsPerBlock)) & ((1 << bitsPerBlock) - 1);
			    rawX = ((pos >> 8) & 0xF);
			    rawY = (pos & 0xF);
			    rawZ = ((pos >> 4) & 0xF);
			    currChunk->addBlock(state, rawX, rawY + 16 * chunkY, rawZ);
			}

			pos++;
		    }
		}
		    
		//Move to start of next block data (if any)
		currOffset = paletteOffset;
	    }
	}
    }

    delete lit;
    delete db;

    cerr << "Total Chunks: " << totalChunks << endl;

    return true;
}
