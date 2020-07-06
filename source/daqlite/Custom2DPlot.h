/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file Custom2DPlot.h
///
/// \brief Creates a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <QPlot/QPlot.h>
#include <logical_geometry/ESSGeometry.h>
#include <chrono>

class Custom2DPlot : public QCustomPlot {
public:
  /// \brief plot needs the configurable plotting options
  Custom2DPlot(Configuration &Config);

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void addData(int phase, std::vector<uint32_t> & Histogram);

private:
  /// \brief updates the image
  void plotDetectorImage(int count);

  // QCustomPlot variables
  QCPColorScale *mColorScale{nullptr};
  QCPColorMap *mColorMap{nullptr};

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  std::vector<uint32_t> HistogramData;

  /// \brief for calculating x, y, z from pixelid
  ESSGeometry * LogicalGeometry;

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
