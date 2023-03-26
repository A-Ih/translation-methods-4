#!/usr/bin/env sh

if [ ! -f ./CMakeLists.txt ]
then
    echo "This script should be launched from the project root"
    exit 1
fi


./debug/generator --grammar_file calculator/grammar --out_dir calculator/
c++ calculator/main.cc -o calculator/out -std=c++17
./calculator/out >pic.dot && dot -Tpng pic.dot >pic.png && open pic.png
