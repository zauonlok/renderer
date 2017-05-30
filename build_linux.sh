#!/bin/bash

OPTS="-o viewer.out -std=c89 -Wall"
SRCS="main.c platform_linux.c geometry.c graphics.c image.c model.c"
LIBS="-lX11"

gcc $OPTS $SRCS $LIBS
