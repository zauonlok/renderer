#!/bin/bash

OPTS="-o ../Viewer -std=c89 -pedantic -Wall -Wextra"
SRCS="main.c platform_macos.m geometry.c graphics.c image.c model.c"
LIBS="-framework Cocoa"

cd renderer
clang $OPTS $SRCS $LIBS
cd ..
