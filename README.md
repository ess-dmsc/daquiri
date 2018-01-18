# DAQuiri
Versatile DAQ engine for physics detectors, event mode and otherwise

- Has variety of histogram types for images, spectra, time-of-flight, activity

## Installation

### Requirements

Supported platforms:
- Centos7 w/ gcc6
- Fedora 25
- OSX (latest?)
- Ubuntu16.04
- Ubuntu17.10

You definitely need these:
- C++ compiler with c++11 support
- CMake
- Qt  (>=5.5)
- conan 1.0 (via pip)

If you want to contribute, you might want these:
- Google Test
- dialog

You also need these, but if you have conan, don't worry about it:
- CMake
- boost
- LibRDKafka
- FlatBuffers
- Eigen3
- [h5cpp](https://github.com/ess-dmsc/h5cpp)


### Build

```
git submodule update --init
mkdir build
cd build
conan remote add ess-dmsc https://api.bintray.com/conan/ess-dmsc/conan
conan install --build=missing ../conanfile.txt
cmake ..
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
