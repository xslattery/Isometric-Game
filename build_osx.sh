#!/bin/bash

DIR_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )";
cd "$DIR_PATH";

OUTPUT="-o build/osx"

CPP_FILES="src/*.cpp src/scenes/*.cpp src/scenes/scene_game/*.cpp"
OBJC_FILES="src/platform/*.mm"

LIBS="-framework Cocoa -framework Quartz -framework OpenGL -l freetype "
INCLUDE_PATH="-I libs/include"
LIB_PATH="-L libs"

DEF="-DPLATFORM_OSX -DDEBUG=0 -DSSE_SUPPORT=1"

clang++ -g -O0 -std=c++14 -arch x86_64 $LIBS $LIB_PATH $INCLUDE_PATH $DEF $OBJC_FILES $CPP_FILES $OUTPUT
