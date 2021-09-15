#include <iostream>
#include "ParseBedrock.h"
#include "leveldb/db.h" 
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/decompress_allocator.h"

using namespace std;

uint32_t parseInt(leveldb::Slice &k, uint32_t offset)
{
	uint32_t result;

	result = 0;
	for(int i = 0; i < 4; i++)
	{
		//Static cast prevents sign extension
		result |= (static_cast<uint8_t>(k[offset + i] << i * 8));
	}

	return result;
}

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

	if(!status.ok())
	{
		cout << "Could not open db folder '" << path << "'" << endl;
		return false;
	}

	lit = db->NewIterator(readOptions);

	int totalChunks = 0;
	for(lit->SeekToFirst(); lit->Valid(); lit->Next())
	{
		leveldb::Slice k, v;
		
		k = lit->key();
		v = lit->value();

		//Keys for chunks are between 9 and 14 bytes
		if(k.size() >= 9 && k.size() <= 14)
		{
			uint32_t x, z, regionID;
			string regionName;

			x = parseInt(k, 0);
			z = parseInt(k, 4);
			regionName = "Overworld";

			if(k.size() == 13 || k.size() == 14)
			{
				regionID = parseInt(k, 8);
				regionName = (regionID == 1 ? "Nether" : "End");
			}

			cerr << "Found " << regionName << " Chunk at ";
			cerr << "(" << x << ", " << z << "): " << endl;
			totalChunks++;
		}
	}

	cerr << "Total Chunks: " << totalChunks << endl;

	return true;
}
