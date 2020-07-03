
#pragma once

#include <Configuration.h>
#include <QPlot/QPlot.h>
#include <logical_geometry/ESSGeometry.h>

class Custom2DPlot : public QCustomPlot {
public:
  Custom2DPlot(Configuration &Config);

  void colorMap(int phase);

  void addData(int phase, std::vector<uint32_t> & Histogram);

private:
  // configure axis rect:
  QCPColorScale *mColorScale{nullptr};
  QCPColorMap *mColorMap{nullptr};
  Configuration &mConfig;

  std::vector<uint32_t> HistogramData;
  ESSGeometry * LogicalGeometry;
};
