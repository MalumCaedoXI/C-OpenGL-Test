#!/bin/sh

mkdir logs
./build/C-OpenGL-Test.exe | tee ./logs/Log.txt
