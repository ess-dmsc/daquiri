#!/bin/bash

pushd $(dirname "${BASH_SOURCE[0]}")/../build

source ./activate_run.sh
./bin/acquire $@
