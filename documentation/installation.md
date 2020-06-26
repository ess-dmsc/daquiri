## Requirements

Supported (continuously tested) platforms:
- Ubuntu 18.04
- macOS High Sierra
- Centos 7

## Installing

You are very likely using this within the ESS data acquisition framework. In
this case you will want to automatically install and update DAQuiri
using [ESS DAQ](https://github.com/ess-dmsc/essdaq)

The details of manually configuring all the dependencies can be found [here](manual_building.md)

## Running

To run daquiri, you can use this convenience script:
```
./utils/daquiri.sh
```
Or, if you need the command-line only utility:
```
./utils/acquire.sh
```
to both of which you can supply whatever parameters you would to the actual programs.

## Time profiling (macOS)
To use Instrument for time profiling Daquiri
```
> cd build
> cmake -G Xcode -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
> open daquiri.xcodeproj
```

In the xcode app use &#8984;-I to build and chose Time Profiler.

 
