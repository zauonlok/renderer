#!/bin/bash

OPTS="-o viewer.out -std=c89 -Wall"
SRCS="main.c platform_macos.m image.c error.c buffer.c geometry.c model.c"
LIBS="-framework Cocoa"

clang $OPTS $SRCS $LIBS
