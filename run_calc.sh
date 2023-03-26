#!/usr/bin/env sh

if [ ! -f ./CMakeLists.txt ]
then
    echo "This script should be launched from the project root"
    exit 1
fi


./debug/generator --grammar_file calculator/grammar --out_dir calculator/
c++ calculator/main.cc -o calculator/out -std=c++17

while read -r line
do
    echo "Testing '$line'..."
    if echo "$line" | ./calculator/out >/dev/null; then
        echo "OK: parser returned 0"
    else
        echo "FAIL: parser failed"
        exit 1
    fi
done <./calculator/samples


echo "Now you can enter a custom expression to parse, I will print the result and draw the AST"
echo "Waiting for your input..."
./calculator/out >pic.dot && dot -Tpng pic.dot >pic.png && open pic.png
