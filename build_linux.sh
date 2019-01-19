#!/bin/bash

OPTS="-o ../Viewer -std=c89 -pedantic -Wall -Wextra -O3"
SRCS="main.c platforms/linux.c core/*.c scenes/*.c shaders/*.c tests/*.c"
LIBS="-lm -lX11"

cd renderer
gcc $OPTS $SRCS $LIBS
cd ..
