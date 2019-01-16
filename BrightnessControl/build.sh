#!/bin/sh
clang++ -std=c++1z -Wall -O3 adjustBrightness.cpp -lX11 -L/usr/local/lib/ -I/usr/local/include/
rm *.gch *# *~ *.core
