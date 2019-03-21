#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")/..

rm -fr build
mkdir -p build
pushd build

function detectos()
{
    cat /etc/centos-release 2>/dev/null | grep CentOS &>/dev/null && SYSTEM=centos
    cat /etc/lsb-release 2>/dev/null    | grep Ubuntu &>/dev/null && SYSTEM=ubuntu
    uname -a | grep Darwin &>/dev/null && SYSTEM=macos
}

detectos

if [[ $SYSTEM == "centos" ]]; then
    conan install --build=boost_filesystem --options boost_filesystem:shared=True \
          --options boost_system:shared=True boost_filesystem/1.69.0@bincrafters/stable || exit 1
    conan install --build=outdated .. || exit 1
    cmake -DCONAN=MANUAL ..
else
    cmake ..
fi

make everything $@
source ./activate_run.sh
make run_tests
