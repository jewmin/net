#!/bin/bash
mkdir -p build/linux_debug
cd build/linux_debug
cmake ../.. -DRELEASE=OFF
make
