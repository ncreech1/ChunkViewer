#!/bin/bash

./ChunkViewer db < examples_input.txt;
jgraph -P terrainX.txt | ps2pdf - | convert -density 300 - -quality 100 terrainX.jpg
jgraph -P terrainZ.txt | ps2pdf - | convert -density 300 - -quality 100 terrainZ.jpg
jgraph -P trees.txt | ps2pdf - | convert -density 300 - -quality 100 trees.jpg
jgraph -P beach.txt | ps2pdf - | convert -density 300 - -quality 100 beach.jpg
jgraph -P beachBedrock.txt | ps2pdf - | convert -density 300 - -quality 100 beachBedrock.jpg
