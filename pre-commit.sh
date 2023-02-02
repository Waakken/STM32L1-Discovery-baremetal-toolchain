#!/usr/bin/bash

set -e
echo "Cleaning first"
make clean
echo "Running make for arm"
make
echo "Running make for x86"
make x86
echo "Running x86 executable"
./main_x86
echo "Doing formating"
make format

echo "Commit hook ok"

set +e
