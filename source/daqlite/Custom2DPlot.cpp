// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Custom2DPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <Custom2DPlot.h>
#include <WorkerThread.h>
#include <algorithm>
#include <assert.h>
#include <fmt/format.h>
#include <string>

Custom2DPlot::Custom2DPlot(Configuration &Config, Projection Proj)
    : mConfig(Config), mProjection(Proj) {

  connect(this, SIGNAL(mouseMove(QMouseEvent *)), this,
          SLOT(showPointToolTip(QMouseEvent *)));
  setAttribute(Qt::WA_AlwaysShowToolTips);

  auto &geom = mConfig.Geometry;

  LogicalGeometry = new ESSGeometry(geom.XDim, geom.YDim, geom.ZDim, 1);
  HistogramData.resize(LogicalGeometry->max_pixel() + 1);

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
  if (mProjection == ProjectionXY) {
    xAxis->setLabel("X");
    yAxis->setLabel("Y");
    mColorMap->data()->setSize(geom.XDim, geom.YDim);
    mColorMap->data()->setRange(QCPRange(0, geom.XDim - 1),
                                QCPRange(0, geom.YDim - 1)); //
  } else if (mProjection == ProjectionXZ) {
    xAxis->setLabel("X");
    yAxis->setLabel("Z");
    mColorMap->data()->setSize(geom.XDim, geom.ZDim);
    mColorMap->data()->setRange(QCPRange(0, geom.XDim - 1),
                                QCPRange(0, geom.ZDim - 1));
  } else {
    xAxis->setLabel("Y");
    yAxis->setLabel("Z");
    mColorMap->data()->setSize(geom.YDim, geom.ZDim);

    mColorMap->data()->setRange(QCPRange(0, geom.YDim - 1),
                                QCPRange(0, geom.ZDim - 1));
  }
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

void Custom2DPlot::setCustomParameters() {
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
QCPColorGradient Custom2DPlot::getColorGradient(std::string GradientName) {
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

std::string Custom2DPlot::getNextColorGradient(std::string GradientName) {
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

void Custom2DPlot::clearDetectorImage() {
  std::fill(HistogramData.begin(), HistogramData.end(), 0);
  addData(HistogramData);
  plotDetectorImage(true);
}

void Custom2DPlot::plotDetectorImage(bool Force) {

  setCustomParameters();

  // if scales match the dimensions (xdim 400, range 0, 399) then cell indexes
  // and coordinates match. PixelId 0 does not exist.
  for (unsigned int i = 1; i < HistogramData.size(); i++) {
    if ((HistogramData[i] != 0) or (Force)) {
      auto xIndex = LogicalGeometry->x(i);
      auto yIndex = LogicalGeometry->y(i);
      auto zIndex = LogicalGeometry->z(i);

      if (mProjection == ProjectionXY) {
        // printf("XY: x,y,z %d, %d, %d: count %d\n", xIndex, yIndex, zIndex,
        // HistogramData[i]);
        mColorMap->data()->setCell(xIndex, yIndex, HistogramData[i]);
      } else if (mProjection == ProjectionXZ) {
        // printf("XZ: x,y,z %d, %d, %d: count %d\n", xIndex, yIndex, zIndex,
        // HistogramData[i]);
        mColorMap->data()->setCell(xIndex, zIndex, HistogramData[i]);
      } else {
        // printf("YZ: x,y,z %d, %d, %d: count %d\n", xIndex, yIndex, zIndex,
        // HistogramData[i]);
        mColorMap->data()->setCell(yIndex, zIndex, HistogramData[i]);
      }
    }
  }

  // rescale the data dimension (color) such that all data points lie in the
  // span visualized by the color gradient:
  mColorMap->rescaleDataRange(true);

  replot();
}

void Custom2DPlot::addData(std::vector<uint32_t> &Histogram) {
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<int64_t, std::nano> elapsed = t2 - t1;

  // Periodically clear the histogram
  //
  int64_t nsBetweenClear = 1000000000LL * mConfig.Plot.ClearEverySeconds;
  if (mConfig.Plot.ClearPeriodic and (elapsed.count() >= nsBetweenClear)) {
    std::fill(HistogramData.begin(), HistogramData.end(), 0);
    t1 = std::chrono::high_resolution_clock::now();
  }

  // Accumulate counts, PixelId 0 does not exist
  for (unsigned int i = 1; i < Histogram.size(); i++) {
    HistogramData[i] += Histogram[i];
  }
  plotDetectorImage(false);
  return;
}

// MouseOver
void Custom2DPlot::showPointToolTip(QMouseEvent *event) {
  int x = this->xAxis->pixelToCoord(event->pos().x());
  int y = this->yAxis->pixelToCoord(event->pos().y());

  setToolTip(QString("%1 , %2").arg(x).arg(y));
}
