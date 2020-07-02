

#include <Custom2DPlot.h>
#include <WorkerThread.h>
#include <string>

Custom2DPlot::Custom2DPlot(Configuration &Config) : mConfig(Config) {
  setInteractions(QCP::iRangeDrag |
                  QCP::iRangeZoom); // this will also allow rescaling the color
                                    // scale by dragging/zooming
  axisRect()->setupFullAxesBox(true);

  // set up the QCPColorMap:
  mColorMap = new QCPColorMap(xAxis, yAxis);

  mColorMap->data()->setSize(
      mConfig.mXDim,
      mConfig.mYDim); // we want the color map to have nx * ny data points
  mColorMap->data()->setRange(QCPRange(0, mConfig.mXDim - 1),
                              QCPRange(0, mConfig.mYDim - 1)); //
  // add a color scale:
  mColorScale = new QCPColorScale(this);
  plotLayout()->addElement(
      0, 1, mColorScale); // add it to the right of the main axis rect
  mColorScale->setType(
      QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels
                         // right (actually atRight is already the default)
  mColorMap->setColorScale(
      mColorScale); // associate the color map with the color scale
  mColorMap->setInterpolate(false);
  mColorMap->setTightBoundary(false);
  mColorScale->axis()->setLabel("Counts");
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
}

void Custom2DPlot::colorMap(int count) {

  std::string Counts = "Counts " + std::to_string(count);
  mColorScale->axis()->setLabel(Counts.c_str());
  // if scales match the dimensions (xdim 400, range 0, 399) then cell indexes
  // and coordinates match.
  // double x, y;
  double z;
  for (int xIndex = 0; xIndex < mConfig.mXDim; ++xIndex) {
    for (int yIndex = 0; yIndex < mConfig.mYDim; ++yIndex) {
      // mColorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
      z = 2.0 * xIndex * count / 10 *
          (qCos(xIndex * 0.1 + count) - qSin(yIndex * 0.1 + count));
      mColorMap->data()->setCell(xIndex, yIndex, z);
    }
  }

  // rescale the data dimension (color) such that all data points lie in the
  // span visualized by the color gradient:
  mColorMap->rescaleDataRange();
}

void Custom2DPlot::addData(int count) {
  colorMap(count);
  replot();
  return;
}
