#!/bin/bash

cmake -S . -B Build/Debug -D CMAKE_BUILD_TYPE=Debug
cmake --build Build/Debug --config Debug
cd Build/Debug/Run
hyprctl dispatch fullscreenstate 0

./MyProject
cd ../../..
