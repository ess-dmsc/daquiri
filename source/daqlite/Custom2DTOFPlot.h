// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Custom2DTOFPlot.h
///
/// \brief Creates (maybe) a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <QPlot/QPlot.h>
#include <chrono>
#include <logical_geometry/ESSGeometry.h>

class Custom2DTOFPlot : public QCustomPlot {
  Q_OBJECT
public:
  /// \brief plot needs the configurable plotting options
  Custom2DTOFPlot(Configuration &Config);

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void addData(std::vector<uint32_t> &Pixels, std::vector<uint32_t> &TOFs);

  /// \brief Support for different gradients
  QCPColorGradient getColorGradient(std::string GradientName);

  /// \brief update plot based on (possibly dynamic) config settings
  void setCustomParameters();

  /// \brief rotate through gradient names
  std::string getNextColorGradient(std::string GradientName);

  /// \brief clears histogram data
  void clearDetectorImage(std::vector<uint32_t> &PixelIDs,
      std::vector<uint32_t> &TOFs);

public slots:
  void showPointToolTip(QMouseEvent *event);

private:
  /// \brief updates the image
  /// \param Force forces updates of histogram data with zero count
  void plotDetectorImage(bool Force);

  // QCustomPlot variables
  QCPColorScale *mColorScale{nullptr};
  QCPColorMap *mColorMap{nullptr};

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  /// \brief allocated according to config in constructor
  #define TOF2DX 512
  #define TOF2DY 512
  uint32_t HistogramData2D[TOF2DX][TOF2DY];

  /// \brief for calculating x, y, z from pixelid
  ESSGeometry *LogicalGeometry;

  // 0 = (x,y), 1 = (x,z), 2 = (y,z)
  int mProjection{0};

  // See colors here
  // https://www.qcustomplot.com/documentation/classQCPColorGradient.html
  std::map<std::string, QCPColorGradient> mGradients{
      {"hot", QCPColorGradient::gpHot},
      {"grayscale", QCPColorGradient::gpGrayscale},
      {"cold", QCPColorGradient::gpCold},
      {"night", QCPColorGradient::gpNight},
      {"candy", QCPColorGradient::gpCandy},
      {"geography", QCPColorGradient::gpGeography},
      {"ion", QCPColorGradient::gpIon},
      {"thermal", QCPColorGradient::gpThermal},
      {"polar", QCPColorGradient::gpPolar},
      {"spectrum", QCPColorGradient::gpSpectrum},
      {"jet", QCPColorGradient::gpJet},
      {"hues", QCPColorGradient::gpHues}};

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
