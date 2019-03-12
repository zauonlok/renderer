#!/bin/bash

OPTS="-o ../Viewer -static -g -std=c89 -pedantic -Wall -Wextra -O3 -flto=thin"
SRCS="main.c platforms/macos.m core/*.c scenes/*.c shaders/*.c tests/*.c"
LIBS="-framework Cocoa"

cd renderer
clang $OPTS $SRCS $LIBS
cd ..
