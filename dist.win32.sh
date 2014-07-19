#!/bin/sh

DIRNAME="Lonely_Ruins"

DATE=$(date +%Y%m%d)
ZIPNAME="$DIRNAME.$DATE.zip"

make -f Makefile.win32 clean
make -f Makefile.win32

mkdir $DIRNAME
cd $DIRNAME
cp -v ../readme.txt ./
cp -v ../build-mingw/game.exe ./game.exe
cp -v ../SDL.dll ./
mkdir data
cp -v ../data/*.bmp data/
cp -v ../data/*.wav data/
cp -v ../data/level_*.txt data/
cd ..

rm $ZIPNAME
zip -r $ZIPNAME $DIRNAME

rm -rf $DIRNAME

