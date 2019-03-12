#!/bin/bash

OPTS="-o ../Viewer -static -g -std=c89 -pedantic -Wall -Wextra -O3 -flto=full"
SRCS="main.c platforms/linux.c core/*.c scenes/*.c shaders/*.c tests/*.c"
LIBS="-lm -lX11"

cd renderer
gcc $OPTS $SRCS $LIBS
cd ..
