// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CustomTofPlot.h
///
/// \brief Creates a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <QPlot/QPlot.h>
#include <chrono>
#include <logical_geometry/ESSGeometry.h>

class CustomTofPlot : public QCustomPlot {
  Q_OBJECT
public:
  /// \brief plot needs the configurable plotting options
  CustomTofPlot(Configuration &Config);

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void addData(std::vector<uint32_t> &Histogram);

  /// \brief update plot based on (possibly dynamic) config settings
  void setCustomParameters();

  ///
  void clearDetectorImage();

private:
  const int TofBinSize{512};
  /// \brief updates the image
  /// \param Force forces updates of histogram data with zero count
  void plotDetectorImage(bool Force);

  // QCustomPlot variables
  QCPGraph *mGraph{nullptr};

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  std::vector<uint32_t> HistogramTofData;

  /// \brief for calculating x, y, z from pixelid
  ESSGeometry *LogicalGeometry;

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
