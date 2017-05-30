#!/bin/bash

OPTS="-o viewer.out -std=c89 -Wall"
SRCS="main.c platform_linux.c image.c buffer.c geometry.c model.c graphics.c"
LIBS="-lX11"

gcc $OPTS $SRCS $LIBS
