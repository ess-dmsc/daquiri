# DAQuiri
Versatile DAQ engine for physics detectors, event mode and otherwise

- Has variety of histogram types for images, spectra, time-of-flight, activity

## Installation

### Requirements

These libraries are expected in default locations:

- boost
- Qt
- librdkafka
- flatbuffers (headers and `flatc` executable)

Tooling
- cmake (minimum tested is 2.8.11)
- C++ compiler with c++11 support

Others (optional)
- Google Test


### Build

Assuming you have `make` and all dependencies in standard locations:
```
git submodule update --init
<path-to-source>/util/config.sh (to select desired components)
cmake <path-to-source>/src
make
```

#### Dependencies in custom locations

Something about Qt...

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
