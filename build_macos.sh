#!/bin/bash

OPTS="-o viewer.out -std=c89 -Wall"
SRCS="main.c platform_macos.m image.c buffer.c geometry.c model.c graphics.c"
LIBS="-framework Cocoa"

clang $OPTS $SRCS $LIBS
