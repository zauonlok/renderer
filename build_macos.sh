#!/bin/bash

OPTS="-o viewer.app -std=c89 -Wall"
SRCS="main.c platform_macos.m image.c error.c"
LIBS="-framework Cocoa"

clang $OPTS $SRCS $LIBS
