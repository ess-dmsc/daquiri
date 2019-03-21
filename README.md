# DAQuiri
Versatile DAQ engine for physics detectors, event mode and otherwise, with a variety of histogram types for images, spectra, time-of-flight, activity.

[![DOI](https://zenodo.org/badge/94489375.svg)](https://zenodo.org/badge/latestdoi/94489375)

![screenshot](screenshot.png)

## Requirements

Supported platforms:
- Ubuntu 18.04
- macOS High Sierra
- Centos 7

## Installing

You are very likely using this within the ESS data acquisition framework. In this case you will want to automatically install and update DAQuiri using [ESS DAQ](https://github.com/ess-dmsc/essdaq)

The gory details of manually configuring all the dependencies can be found [here](manual_building.md)

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

## The future

You can see current tickets and their priorities on the [Kanban board](https://github.com/ess-dmsc/daquiri/projects/1)

Please send any feature requests you have to (martin.shetty@esss.se).
