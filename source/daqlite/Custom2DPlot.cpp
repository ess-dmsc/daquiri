

#include <Custom2DPlot.h>
#include <WorkerThread.h>
#include <string>

Custom2DPlot::Custom2DPlot(Configuration &Config) : mConfig(Config) {

  LogicalGeometry = new ESSGeometry(mConfig.Geometry.XDim, mConfig.Geometry.YDim, 1, 1);
  HistogramData.resize(LogicalGeometry->nx() * LogicalGeometry->ny());

  setInteractions(QCP::iRangeDrag |
                  QCP::iRangeZoom); // this will also allow rescaling the color
                                    // scale by dragging/zooming
  axisRect()->setupFullAxesBox(true);

  // set up the QCPColorMap:
  mColorMap = new QCPColorMap(xAxis, yAxis);

  // we want the color map to have nx * ny data points
  mColorMap->data()->setSize(mConfig.Geometry.XDim, mConfig.Geometry.YDim);

  mColorMap->data()->setRange(QCPRange(0, mConfig.Geometry.XDim - 1),
                              QCPRange(0, mConfig.Geometry.YDim - 1)); //
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
  mColorScale->axis()->setLabel("Runtime"); // not change in colorMap too

  // set the color gradient of the color map to one of the presets:
  mColorMap->setGradient(QCPColorGradient::gpPolar);
  // we could have also created a QCPColorGradient instance and added own colors
  // to the gradient, see the documentation of QCPColorGradient for what's
  // possible.

  // make sure the axis rect and color scale synchronize their bottom and top
  // margins (so they line up):
  QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
  axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
  mColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

  // rescale the key (x) and value (y) axes so the whole color map is visible:
  rescaleAxes();

  t1 = std::chrono::high_resolution_clock::now();
}

void Custom2DPlot::colorMap(int count) {

  std::string LabelRuntime = "Runtime " + std::to_string(count);
  mColorScale->axis()->setLabel(LabelRuntime.c_str());
  // if scales match the dimensions (xdim 400, range 0, 399) then cell indexes
  // and coordinates match.
  for (int i = 0; i < HistogramData.size(); i++) {
      auto xIndex = LogicalGeometry->x(i);
      auto yIndex = LogicalGeometry->y(i);
      mColorMap->data()->setCell(xIndex, yIndex, HistogramData[i]);
  }

  // rescale the data dimension (color) such that all data points lie in the
  // span visualized by the color gradient:
  mColorMap->rescaleDataRange();
}

void Custom2DPlot::addData(int count, std::vector<uint32_t> & Histogram) {
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<int64_t,std::nano> elapsed = t2 - t1;

  if ( mConfig.Plot.ClearPeriodic and (elapsed.count() >= 1000000000ULL * mConfig.Plot.ClearEverySeconds)) {
    printf("Clear histogram! %llu / %llu\n", elapsed.count(), 1000000000ULL * mConfig.Plot.ClearEverySeconds);
    std::fill(HistogramData.begin(), HistogramData.end(), 0);
    t1 = std::chrono::high_resolution_clock::now();
  }

  for (int i = 1; i < Histogram.size(); i++) {
    HistogramData[i] += Histogram[i];
  }
  colorMap(count);
  replot();
  return;
}
