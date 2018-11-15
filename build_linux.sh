#!/bin/bash

OPTS="-o ../Viewer -std=c89 -pedantic -Wall -Wextra -O3"
SRCS="main.c platform_linux.c camera.c geometry.c graphics.c image.c mesh.c shaders/phong_shader.c"
LIBS="-lm -lX11"

cd renderer
gcc $OPTS $SRCS $LIBS
cd ..
