#!/bin/bash

DEFS="-D_POSIX_C_SOURCE=200809L"
OPTS="-o ../Viewer -std=c89 -Wall -Wextra -pedantic -O3 -flto -ffast-math"
SRCS="main.c platforms/linux.c core/*.c scenes/*.c shaders/*.c tests/*.c"
LIBS="-lm -lX11"

cd renderer && gcc $DEFS $OPTS $SRCS $LIBS && cd ..
