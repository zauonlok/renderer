#!/bin/bash

OPTS="-std=c89 -Wall -Wextra -pedantic -O3 -flto -ffast-math"
SRCS="main.c platforms/macos.m core/*.c scenes/*.c shaders/*.c tests/*.c"
LIBS="-framework Cocoa"

cd renderer && clang -o ../Viewer $OPTS $SRCS $LIBS && cd ..
