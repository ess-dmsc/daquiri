// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Custom2DPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <CustomAMOR2DTOFPlot.h>
#include <WorkerThread.h>
#include <algorithm>
#include <assert.h>
#include <fmt/format.h>
#include <string>

CustomAMOR2DTOFPlot::CustomAMOR2DTOFPlot(Configuration &Config)
    : mConfig(Config) {

  if ((not (mConfig.Geometry.YDim <= TOF2DY) or
      (not (mConfig.TOF.BinSize <= TOF2DX)))) {
    throw(std::runtime_error("2D TOF histogram size mismatch"));
  }

  memset(HistogramData2D, 0, sizeof(HistogramData2D));


  connect(this, SIGNAL(mouseMove(QMouseEvent *)), this,
          SLOT(showPointToolTip(QMouseEvent *)));
  setAttribute(Qt::WA_AlwaysShowToolTips);

  auto &geom = mConfig.Geometry;

  LogicalGeometry = new ESSGeometry(geom.XDim, geom.YDim, geom.ZDim, 1);
  //HistogramData.resize(LogicalGeometry->max_pixel() + 1);

  // this will also allow rescaling the color scale by dragging/zooming
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  axisRect()->setupFullAxesBox(true);

  // set up the QCPColorMap:
  yAxis->setRangeReversed(true);
  yAxis->setSubTicks(true);
  xAxis->setSubTicks(false);
  xAxis->setTickLabelRotation(90);

  mColorMap = new QCPColorMap(xAxis, yAxis);

  // we want the color map to have nx * ny data points
  xAxis->setLabel("TOF");
  yAxis->setLabel("Y");
  mColorMap->data()->setSize(mConfig.TOF.BinSize, geom.YDim);
  mColorMap->data()->setRange(QCPRange(0, mConfig.TOF.MaxValue),
                              QCPRange(0, mConfig.Geometry.YDim)); //

  // add a color scale:
  mColorScale = new QCPColorScale(this);

  // add it to the right of the main axis rect
  plotLayout()->addElement(0, 1, mColorScale);

  // scale shall be vertical bar with tick/axis labels
  // right (actually atRight is already the default)
  mColorScale->setType(QCPAxis::atRight);

  // associate the color map with the color scale
  mColorMap->setColorScale(mColorScale);
  mColorMap->setInterpolate(mConfig.Plot.Interpolate);
  mColorMap->setTightBoundary(false);
  mColorScale->axis()->setLabel("Counts");

  setCustomParameters();

  // make sure the axis rect and color scale synchronize their bottom and top
  // margins (so they line up):
  QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
  axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
  mColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

  // rescale the key (x) and value (y) axes so the whole color map is visible:
  rescaleAxes();

  t1 = std::chrono::high_resolution_clock::now();
}

void CustomAMOR2DTOFPlot::setCustomParameters() {
  // set the color gradient of the color map to one of the presets:
  QCPColorGradient Gradient(getColorGradient(mConfig.Plot.ColorGradient));

  if (mConfig.Plot.InvertGradient) {
    Gradient = Gradient.inverted();
  }

  mColorMap->setGradient(Gradient);

  if (mConfig.Plot.LogScale) {
    mColorMap->setDataScaleType(QCPAxis::stLogarithmic);
  } else {
    mColorMap->setDataScaleType(QCPAxis::stLinear);
  }
}

// Try the user supplied gradient name, then fall back to 'hot' and
// provide a list of options
QCPColorGradient CustomAMOR2DTOFPlot::getColorGradient(std::string GradientName) {
  if (mGradients.find(GradientName) != mGradients.end()) {
    return mGradients.find(GradientName)->second;
  } else {
    fmt::print("Gradient {} not found, using 'hot' instead.\n", GradientName);
    fmt::print("Supported gradients are: ");
    for (auto &Gradient : mGradients) {
      fmt::print("{} ", Gradient.first);
    }
    fmt::print("\n");
    return mGradients.find("hot")->second;
  }
}

std::string CustomAMOR2DTOFPlot::getNextColorGradient(std::string GradientName) {
  bool SaveFirst{true};
  bool SaveNext{false};
  std::string RetVal;

  for (auto &Gradient : mGradients) {
    if (SaveFirst) {
      RetVal = Gradient.first;
      SaveFirst = false;
    }
    if (SaveNext) {
      RetVal = Gradient.first;
      SaveNext = false;
    }
    if (Gradient.first == GradientName) {
      SaveNext = true;
    }
  }
  return RetVal;
}

void CustomAMOR2DTOFPlot::clearDetectorImage(std::vector<uint32_t> &PixelIDs,
    std::vector<uint32_t> &TOFs) {
  PixelIDs.clear();
  TOFs.clear();
  memset(HistogramData2D, 0, sizeof(HistogramData2D));
  plotDetectorImage(true);
}

void CustomAMOR2DTOFPlot::plotDetectorImage(bool Force) {
  setCustomParameters();

  for (unsigned int y = 0; y < 352; y++) {
    for (unsigned int x = 0; x < 512; x++) {
      if ((HistogramData2D[x][y] == 0) and (not Force)) {
        continue;
      }
      //printf("debug x %u, y %u, z %u\n", x, y, HistogramData2D[x][y]);
      mColorMap->data()->setCell(x, y, HistogramData2D[x][y]);
    }
  }

  // rescale the data dimension (color) such that all data points lie in the
  // span visualized by the color gradient:
  mColorMap->rescaleDataRange(true);

  replot();
}

void CustomAMOR2DTOFPlot::addData(std::vector<uint32_t> &PixelIDs,
    std::vector<uint32_t> &TOFs) {
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<int64_t, std::nano> elapsed = t2 - t1;

  // Accumulate counts, PixelId 0 does not exist
  if (PixelIDs.size() == 0) {
    return;
  }

  for (uint i = 0; i < PixelIDs.size(); i++) {
    if (PixelIDs[i] == 0) {
      continue;
    }
    int tof  = TOFs[i];
    int yvals = (PixelIDs[i] - 1)/mConfig.Geometry.XDim;
    HistogramData2D[tof][yvals]++;
  }
  plotDetectorImage(false);
  return;
}

// MouseOver
void CustomAMOR2DTOFPlot::showPointToolTip(QMouseEvent *event) {
  int x = this->xAxis->pixelToCoord(event->pos().x());
  int y = this->yAxis->pixelToCoord(event->pos().y());

  setToolTip(QString("%1 , %2").arg(x).arg(y));
}
