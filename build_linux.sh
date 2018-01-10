#!/bin/bash

OPTS="-o ../Viewer -std=c89 -pedantic -Wall -Wextra -D_POSIX_C_SOURCE=200809L -O3"
SRCS="main.c platform_linux.c drawing.c geometry.c graphics.c image.c model.c"
LIBS="-lm -lX11"

cd renderer
gcc $OPTS $SRCS $LIBS
cd ..
