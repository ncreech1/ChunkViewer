CC = g++

INCLUDE = ./leveldb-mcpe/include

CFLAGS = -std=c++17 -DDLLX= -I. -I$(INCLUDE)

LIBS = -L./leveldb-mcpe/out-static -lleveldb -lz -lpthread

OBJS = ChunkViewer.o ParseBedrock.o

ChunkViewer: $(OBJS)
	$(CC) $(CFLAGS) -o ChunkViewer $(OBJS) $(LIBS)

ChunkViewer.o: ChunkViewer.cpp ParseBedrock.h
	$(CC) $(CFLAGS) -c ChunkViewer.cpp $(LIBS)

ParseBedrock.o: ParseBedrock.cpp ParseBedrock.h
	$(CC) $(CFLAGS) -c ParseBedrock.cpp $(LIBS)
