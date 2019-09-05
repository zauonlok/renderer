#!/bin/bash

OPTS="-o ../Viewer -std=c89 -Wall -Wextra -pedantic -O3 -flto -ffast-math"
SRCS="main.c platforms/macos.m core/*.c shaders/*.c scenes/*.c tests/*.c"
LIBS="-framework Cocoa"

cd renderer && clang $OPTS $SRCS $LIBS && cd ..
