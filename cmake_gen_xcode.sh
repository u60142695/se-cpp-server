#!/bin/sh

rm -Rf cmake_xcode/
mkdir cmake_xcode/
cd cmake_xcode/
cmake -G "Xcode" ..
cd ..
