#!/bin/bash

OPTS="-o ../Viewer -std=c89 -pedantic -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Ofast -flto"
SRCS="main.c platforms/linux.c core/*.c scenes/*.c shaders/*.c tests/*.c"
LIBS="-lm -lX11"

cd renderer && gcc $OPTS $SRCS $LIBS && cd ..
