#!/usr/bin/env sh

if [ ! -f ./CMakeLists.txt ]
then
    echo "This script should be launched from the project root"
    exit 1
fi


./debug/generator --grammar_file lambda/grammar --out_dir lambda/
c++ lambda/main.cc -o lambda/out -std=c++17

while read -r line
do
    echo "Testing '$line'..."
    if echo "$line" | ./lambda/out >/dev/null; then
        echo "OK: parser returned 0"
    else
        echo "FAIL: parser failed"
        exit 1
    fi
done <./lambda/examples

while read -r line
do
    echo "Testing '$line'..."
    if echo "$line" | ./lambda/out >/dev/null; then
        echo "FAIL: error expected"
        exit 1
    else
        echo "OK: parser failed"
    fi
done <./lambda/invalid-examples

echo "==============================================================="
echo "==========================SUCCESS=============================="
echo "==============================================================="

./lambda/out >pic.dot && dot -Tpng pic.dot >pic.png && open pic.png

