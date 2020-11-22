#!/bin/bash

rm -Rf cmake_lin64
mkdir cmake_lin64
cd cmake_lin64

cmake -G "Unix Makefiles" ..
