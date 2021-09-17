CC = g++

INCLUDE = ./leveldb-mcpe/include

CFLAGS = -std=c++17 -DDLLX= -I. -I$(INCLUDE)

LIBS = -L./leveldb-mcpe/out-static -lleveldb -lz -lpthread

OBJS = ChunkViewer.o ParseBedrock.o NBTTag.o WorldData.o Chunk.o

ChunkViewer: $(OBJS)
	$(CC) $(CFLAGS) -o ChunkViewer $(OBJS) $(LIBS)

ChunkViewer.o: ChunkViewer.cpp ParseBedrock.h NBTTag.h WorldData.h Chunk.h
	$(CC) $(CFLAGS) -c ChunkViewer.cpp $(LIBS)

ParseBedrock.o: ParseBedrock.cpp NBTTag.h WorldData.h Chunk.h
	$(CC) $(CFLAGS) -c ParseBedrock.cpp $(LIBS)

NBTTag.o: NBTTag.cpp NBTTag.h
	$(CC) $(CFLAGS) -c NBTTag.cpp $(LIBS)

WorldData.o: WorldData.cpp Chunk.h
	$(CC) $(CFLAGS) -c WorldData.cpp $(LIBS)

Chunk.o: Chunk.cpp NBTTag.h
	$(CC) $(CFLAGS) -c Chunk.cpp $(LIBS)
