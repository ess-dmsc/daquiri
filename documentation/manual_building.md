# Detailed building instructions

In case you want to do everything manually, here are some of the gory details.

## Requirements

Supported platforms:
- Ubuntu 18.04
- macOS High Sierra
- Centos 7

You definitely need these:
- git
- recent C++ compiler
- CMake 3.0
- Qt5
- conan 1.0 (via pip)

## Conan setup
To avoid C++ dependency hell, we use `conan`. The following repos need to be added:
```
conan remote add conancommunity https://api.bintray.com/conan/conan-community/conan
conan remote add conan-transit https://api.bintray.com/conan/conan/conan-transit
conan remote add ess-dmsc https://api.bintray.com/conan/ess-dmsc/conan
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
```
If you are on a `linux` sytem you also need to ensure that `conan` builds everything using the c++11 standard. Edit your `~/.conan/profiles/default` to replace `compiler.libcxx=libstdc++` with `compiler.libcxx=libstdc++11`.
If said file does not exists, you are likely yet to run `conan` for the first time. Do the following to generate the above-mentioned file:
```
conan profile new --detect default
```

## Installation of Qt

There is currently no fully portable conan package of Qt so it should be installed in
a platform-specific way. For each platform it's the following:

- Ubuntu: `qt5-default` package from apt
- Centos: `qt5-qtbase-devel` package from yum
- macOS: whatever default qt package from brew

It is easiest if you have Qt installed via brew or apt.

If your package manager did not provide Qt5 and you used Qt's web installer, you may need to set:
```
CMAKE_PREFIX_PATH=/somepath/Qt/5.5/gcc_64
```
either just prior to `cmake` or in your `~/.profile`


## Building

The usual cmake ritual

```
git clone https://github.com/ess-dmsc/daquiri.git
cd daquiri
mkdir build
cd build
cmake ..
make
```

However, for convenience you also have the following for the first build

```
git clone https://github.com/ess-dmsc/daquiri.git
cd daquiri
./utils/first_build.sh
```

When you want to update to latest version, from `daquiri` directory, run:
```
./utils/update_build.sh
```


## Running

Before you run `daquiri` or `acquire`, you need to activate the conan-generated virtual environment by sourcing `activate_run.sh`. If you are inside your build directory, do the following:

```
source ./activate_run.sh
./bin/daquiri
```
Otherwise, you have the convenience scripts
```
./utils/daquiri.sh
```
and
```
./utils/acquire.sh
```
to which you can supply whatever parameters you would to the actual programs


