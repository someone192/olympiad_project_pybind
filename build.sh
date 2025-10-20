#!/bin/bash
cd "$(dirname "$0")"
rm -rf build                                                                                                               
cmake -S . -B build -DPYBIND11_FINDPYTHON=ON
cmake --build build
echo "Build complete! Test: PYTHONPATH=build python ../test.py"