#!/bin/bash

OPTS="-o ../Viewer -std=c89 -D_POSIX_C_SOURCE=199309L -Wall"
SRCS="main.c platform_linux.c geometry.c graphics.c image.c model.c"
LIBS="-lm -lX11"

cd renderer
gcc $OPTS $SRCS $LIBS
cd ..
