#!/usr/bin/env bash

build_dir=../../build

if [ ! -d "$build_dir" ]; then
  mkdir $build_dir
fi
cd $build_dir
g++ -g -std=c++14 ../handmade/code/linux_handmade.cpp -lSDL2 -o linux_handmade
cd -
