#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")/..

rm -fr build
mkdir -p build
pushd build

conan install --build=boost_filesystem --options boost_filesystem:shared=True \
      --options boost_system:shared=True boost_filesystem/1.69.0@bincrafters/stable || exit 1
conan install --build=outdated .. || exit 1

cmake -DCONAN=MANUAL ..
make $@
make all_tests $@
source ./activate_run.sh
./tests/unit_tests
