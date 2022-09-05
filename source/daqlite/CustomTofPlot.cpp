// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CustomTofPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <CustomTofPlot.h>
#include <WorkerThread.h>
#include <algorithm>
#include <assert.h>
#include <fmt/format.h>
#include <string>

CustomTofPlot::CustomTofPlot(Configuration &Config) : mConfig(Config) {

  auto &geom = mConfig.Geometry;

  LogicalGeometry = new ESSGeometry(geom.XDim, geom.YDim, geom.ZDim, 1);

  HistogramTofData.resize(mConfig.TOF.BinSize);

  // this will also allow rescaling the color scale by dragging/zooming
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  axisRect()->setupFullAxesBox(true);

  // set up the QCPColorMap:
  yAxis->setRangeReversed(false);
  yAxis->setSubTicks(true);
  xAxis->setSubTicks(false);
  xAxis->setTickLabelRotation(90);

  mGraph = new QCPGraph(xAxis, yAxis);
  // mGraph->setLineStyle(QCPGraph::lsNone);
  mGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
  mGraph->setLineStyle(QCPGraph::lsStepCenter);
  mGraph->  setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

  // we want the color map to have nx * ny data points

  if (mConfig.Plot.XAxis.empty()) {
    xAxis->setLabel("TOF (us)");
  } else {
    xAxis->setLabel(mConfig.Plot.XAxis.c_str());
  }

  yAxis->setLabel("Counts");
  xAxis->setRange(0, 50000);

  setCustomParameters();

  t1 = std::chrono::high_resolution_clock::now();
}

void CustomTofPlot::setCustomParameters() {
  if (mConfig.Plot.LogScale) {
    yAxis->setScaleType(QCPAxis::stLogarithmic);
  } else {
    yAxis->setScaleType(QCPAxis::stLinear);
  }
}

void CustomTofPlot::plotDetectorImage(bool Force) {
  setCustomParameters();
  mGraph->data()->clear();
  uint32_t MaxY{0};
  for (unsigned int i = 0; i < HistogramTofData.size(); i++) {
    if ((HistogramTofData[i] != 0) or (Force)) {
      uint32_t x = i * mConfig.TOF.MaxValue / mConfig.TOF.BinSize;
      uint32_t y = HistogramTofData[i];
      if (y > MaxY) {
        MaxY = y;
      }
      mGraph->addData(x, y);
    }
  }

  // yAxis->rescale();
  if (mConfig.TOF.AutoScaleX) {
    xAxis->setRange(0, mConfig.TOF.MaxValue * 1.05);
  }
  if (mConfig.TOF.AutoScaleY){
    yAxis->setRange(0, MaxY * 1.05);
  }
  replot();
}

void CustomTofPlot::addData(std::vector<uint32_t> &Histogram) {
  // printf("addData (TOF) Histogram size %lu\n", Histogram.size());
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<int64_t, std::nano> elapsed = t2 - t1;

  // Periodically clear the histogram
  //
  int64_t nsBetweenClear = 1000000000LL * mConfig.Plot.ClearEverySeconds;
  if (mConfig.Plot.ClearPeriodic and (elapsed.count() >= nsBetweenClear)) {
    std::fill(HistogramTofData.begin(), HistogramTofData.end(), 0);
    t1 = std::chrono::high_resolution_clock::now();
  }

  // Accumulate counts, PixelId 0 does not exist
  for (unsigned int i = 1; i < Histogram.size(); i++) {
    HistogramTofData[i] += Histogram[i];
  }
  plotDetectorImage(false);
  return;
}

void CustomTofPlot::clearDetectorImage() {
  std::fill(HistogramTofData.begin(), HistogramTofData.end(), 0);
  addData(HistogramTofData);
  plotDetectorImage(true);
}
