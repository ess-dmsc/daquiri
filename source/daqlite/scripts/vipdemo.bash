#!/bin/bash

../../../build/bin/daqlite -f ../configs/vipfreia.json &
../../../build/bin/daqlite -f ../configs/vipnmx.json &
../../../build/bin/daqlite -f ../configs/viploki.json &
../../../build/bin/daqlite -f ../configs/vipcspec3d.json &
../../../build/bin/daqlite -f ../configs/vipdream.json
