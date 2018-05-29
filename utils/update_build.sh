#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")/..

git pull
git submodule update
mkdir -p build
pushd build
cmake ..
make $@
make all_tests $@
source ./activate_run.sh
make run_tests
