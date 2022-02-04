#!/bin/bash

../../../build/bin/daqlite -f ../configs/vipdemo/freia.json &
../../../build/bin/daqlite -f ../configs/vipdemo/nmx.json &
../../../build/bin/daqlite -f ../configs/vipdemo/loki.json &
../../../build/bin/daqlite -f ../configs/vipdemo/cspec3d.json &
../../../build/bin/daqlite -f ../configs/vipdemo/dream.json
