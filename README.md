# ChunkViewer

ChunkViewer is a program that utilizes the leveldb-mcpe library from Mojang and the Snappy decompression library to analyze terrain data from the game Minecraft. The program takes the *db* folder for a Minecraft world save and a series of commands on standard input, and outputs a Jgraph script to plot the data. 

Minecraft world data is divided into "chunks" which are 16x16 block segements stretching from a y coordiante of 0 to the world height limit of 256. These chunks are further divided into "sub-chunks" of 16x16x16 block segements. The ChunkViewer program reads this data from the *db* file, loads it into memory, and then takes commands to analyze the data. Air blocks are transparent on the graphs.

**NOTE:** This program works only on Minecraft Bedrock version 1.17.0 (Caves and Cliffs Part I Update). After the recent update (Part II), the chunk storage format changed significantly yet again, and there is not yet public information on the changes, so new worlds post 1.17.0 (and updated chunks in existing worlds) will not be read. The *db* folder included in this repository was created in version 1.17.0, but a few chunks will still be skipped since opening the world post-update modified a few chunks. However, most of the data is still fully readable. Also, I did not prune every block in Minecraft for their naming schemes. Mojang does not seem to make the texture names for their blocks consistent, so I had to hard-code the important ones. As such, a number of blocks will likely display without a proper texture.

### Examples
The following are some examples outputs for the *db* folder included in this repository:

#### Chunk data from two different angles
The target block for the following two graphs is the light blue one (diamond) in the center of this image. I have highlighted it on the graph outputs.

![terrain_axis](https://user-images.githubusercontent.com/59804295/134987262-ba60927e-2157-428b-9d90-33340dc78be8.png)

This is the graph of the x-axis (y = 8 to 72) of the chunk slice looking north, which is the direction of the camera in the first image. You can identify the diorite blocks on the top right, and the air pocket and coal blocks on the bottom left:

![terrainX](https://user-images.githubusercontent.com/59804295/134988408-4b0e8286-97ae-4941-ac0e-bc8d5250aebe.jpg)

This is the graph of the z-axis (y = 8 to 72) of the chunk slice looking east, which is the direction of the camera in the first image, rotated 90 degrees clockwise:

![terrainZ](https://user-images.githubusercontent.com/59804295/134988413-f20ca069-591d-4ea5-aa46-3db3734ea1d9.jpg)

Here are some unique pieces of this target's chunk slice so you can identify them in the corresponding graphs:

(some andesite and diorite blocks from the x-axis graph, around y = 56)

![terrain_x1](https://user-images.githubusercontent.com/59804295/134987562-cd1a1d2b-977f-41fb-aac2-65dfbcbabfe5.png)

(the trees at the top of the z-axis graph, around y = 72)

![terrain_ztrees](https://user-images.githubusercontent.com/59804295/134989454-aa57e75f-c567-4faf-9785-9fe7345f42c0.png)


#### Terrain above ground
For this graph, I did not label the target block. However, you can easily pick out the tree sitting on top of the dirt block, and the relative gaps between the tree line in the following in-game images:

![trees](https://user-images.githubusercontent.com/59804295/134991323-c1b00fda-8b6c-44e9-82bc-c6a5939c2fb0.jpg)
![trees1](https://user-images.githubusercontent.com/59804295/134987337-fb6ceb2a-736a-4de7-b29d-3c935666e220.png)
![trees2](https://user-images.githubusercontent.com/59804295/134987345-dea48813-1d51-4ab3-a1fc-6f7ecf355112.png)


### Compilation
Compile the program by running the makefile, which will also run the test script *gen_examples.sh* and generate five example *.jpg* graphs:
```
make
```

### General Usage
Minecraft terrain data is stored inside a folder named *db* in the corresponding save folder. Copy this folder to the directory containing the ChunkViewer executable and run
```
./ChunkViewer db
```
to load in the chunk data. The program will output *Total Chunks: X* after it has finished, and prompt the user to input commands on standard input with a *cv>* prompt.
The command usage can be found by typing the following commands:
```
cv> help what
what block [(x,y,z)] [-d overworld|nether|end]
cv> help graph
graph block [(x,y,z)] [filename] [-d overworld|nether|end] [-f n|s|e|w] [-ymin int] [-ymax int] 
```
The *what* command outputs basic data about a block, and the *graph* command takes a target block and file name and outputs a Jgraph script for the block's chunk slice. These commands can, of course, be automated by redirecting a file with commands to standard input, or running a shell script to redirect commands and convert to the desired formats.

#### Flags
*-d*: The Minecraft dimension the target block is in (default: overworld)

*-f*: The direction to view the chunk slice from (north, south, east, or west) (default: n)

*-ymin*: The starting y coordinate for the y-axis (default: minimum y for the target block's sub-chunk)

*-ymax*: The ending y coordinate for the y-axis (default: maximum y for the target block's sub-chunk)


#### Script to PDF and JPG
In order to view the graph as a *.pdf*, run the following command using the Jgraph script output:
```
jgraph -P script_name.txt | ps2pdf - pdf_name.pdf
```
For a *.jpg*, the following command works best:
```
jgraph -P script_name.txt | ps2pdf - | convert -density 300 - -quality 100 jpg_name.jpg
```

### Credit
This repository was very helpful as a head-start in utilizing the leveldb-mcpe library, but it is heavily outdated. Some of my code was taken from this repository, but I had to write basically everything from scratch anyway: (https://github.com/mmccoo/minecraft_mmccoo)

This is the leveldb-mcpe github from Mojang: (https://github.com/Mojang/leveldb-mcpe)

This is an update by a Mojang developer about the chunk format for the Aquatic Update, but as of the week of September 19th, 2021, this format (and thus my program) is obsolete: (https://gist.github.com/Tomcc/a96af509e275b1af483b25c543cfbf37#the-new-subchunk-format)

This is the Minecraft wiki page on the sub-chunk format (https://minecraft.fandom.com/wiki/Bedrock_Edition_level_format#1.0)

### Contact Information
Please email any questions or concerns (bugs, etc.) to Nicholas Creech at ncreech1@vols.utk.edu.
