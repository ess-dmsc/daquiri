#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")/..

git pull
mkdir -p build
pushd build
cmake ..
make $@
make all_tests $@
source ./activate_run.sh
./tests/unit_tests
