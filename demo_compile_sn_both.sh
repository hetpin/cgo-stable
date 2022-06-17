#!/bin/bash  
echo "Shell script to compile demo cgraph"
# rm *.fits *.ppm *.o *.gch*
sh ./run_clean.sh
gcc -c fits_read.c fits_read.h mto/main.h -lgsl -lblas -lcfitsio -w -fopenmp
gcc -c mto/background.c mto/background.h -lgsl -fopenmp -w
gcc -c mto/filter.c mto/filter.h -w -fopenmp
g++ -c ragraph.cpp ragraph.h -w -fopenmp
g++ -c utils.h -w -fopenmp -std=c++11
g++ -fopenmp -c cgraph.cpp cgraph.h -std=c++11 -w 
g++ -c Tile.cpp Tile.h -std=c++11 -w -fopenmp
g++ -c cgraphdemo.cpp -std=c++11 -fopenmp -w

g++ -o demo_sn_both cgraphdemo.o cgraph.o ragraph.o fits_read.o background.o Tile.o filter.o -lgsl -lblas -lcfitsio -fopenmp -std=c++11
# ./demo im1.fits im1.fits im1.fits