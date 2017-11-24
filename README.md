# DAQuiri
Versatile DAQ engine for physics detectors, event mode and otherwise

- Has variety of histogram types for images, spectra, time-of-flight, activity

## Installation

### Requirements

Platforms
- Ubuntu (tested)
- OSX (tested)
- Windows (theoretically? someone please try this :))

These libraries are expected in default locations:

- boost
- Qt  (>=5.5)
- librdkafka
- flatbuffers (headers and `flatc` executable)
- eigen3
- [h5cpp](https://github.com/ess-dmsc/h5cpp)

Tooling
- cmake (minimum tested is 2.8.11)
- C++ compiler with c++11 support
- dialog

Others (optional)
- Google Test


### Build

Assuming you have `make` and all dependencies in standard locations:
```
git submodule update --init
util/config.sh (to select desired components)
mkdir build
cd build
cmake ../src
make
```

#### Dependencies in custom locations

If your package manager does not provide Qt5 and you used Qt's web installer instead, you may need to set:
```
CMAKE_PREFIX_PATH=/somepath/Qt/5.5/gcc_64
```
either just prior to `cmake` or in your `~/.profile`

### Tests

Run
```
./tests/daquiri_tests
```

#### Tests with actual traffic

## Performance

## Creating new consumers and producers

## Features Coming Soon

## For the future

Please send any feature requests you have (martin.shetty@esss.se).
