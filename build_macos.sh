#!/bin/bash

OPTS="-o ../Viewer -std=c89 -pedantic -Wall -Wextra -O3 -flto"
SRCS="main.c platforms/macos.m core/*.c scenes/*.c shaders/*.c tests/*.c"
LIBS="-framework Cocoa"

cd renderer && clang $OPTS $SRCS $LIBS && cd ..
