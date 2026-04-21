#!/usr/bin/env sh
gcc -o KeySound.exe keysound.c -mwindows -static -lwinmm -finput-charset=UTF-8 -fexec-charset=UTF-8