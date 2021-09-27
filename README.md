# ChunkViewer

ChunkViewer is a program that utilizes the leveldb-mcpe library from Mojang and the Snappy decompression library to analyze terrain data from the game Minecraft. The program takes the db folder for a Minecraft world save and a series of commands on standard input, and outputs a Jgraph script to plot the data.

**Note:** This program works only on Minecraft Bedrock version 1.6.0 (Aquatic Update). After this version, the chunk storage format changed significantly and there is not yet public information on the changes, so new worlds post 1.6.0 (and updated chunks in existing worlds) will not be read.

### Examples
The following are some examples outputs for the db folder included in this repository:
###### Chunk data from two different angles
###### Terrain above ground
