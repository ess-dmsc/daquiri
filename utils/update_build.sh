#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")/..

git pull
git submodule update
mkdir -p build
pushd build
rm -fr *
cmake ..
make -j
make -j unit_tests
