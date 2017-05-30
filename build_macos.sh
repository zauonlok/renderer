#!/bin/bash

OPTS="-o viewer.out -std=c89 -Wall"
SRCS="main.c platform_macos.m geometry.c graphics.c image.c model.c"
LIBS="-framework Cocoa"

clang $OPTS $SRCS $LIBS
