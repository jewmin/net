#!/bin/bash

config=Debug
tag=OFF

while [ $# -ne 0 ]; do
	if [ "$1" == "debug" ]; then
		config=Debug
	elif [ "$1" == "release" ]; then
		config=Release
	fi
	shift
done

if [ "$config" == "Release" ]; then
	tag=ON
fi

mkdir -p build/$config
cd build/$config
cmake ../.. -DRELEASE=$tag
cmake --build . --config $config
cd ../..
