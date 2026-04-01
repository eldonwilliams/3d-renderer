#! /bin/bash

# Build File

gcc main.c tigr.c egraphics.c input.c utils.c -o build -framework OpenGL -framework Cocoa
chmod +x build
./build
rm build