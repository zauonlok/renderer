#!/bin/bash

OPTS="-o ../Viewer -std=c89 -Wall"
SRCS="main.c platform_linux.c geometry.c graphics.c image.c model.c"
LIBS="-lX11"

cd renderer
gcc $OPTS $SRCS $LIBS
cd ..
