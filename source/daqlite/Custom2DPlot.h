
#pragma once

#include <Configuration.h>
#include <QPlot/QPlot.h>

class Custom2DPlot : public QCustomPlot {
public:
  Custom2DPlot(Configuration &Config);

  void colorMap(int phase);

  void addData(int phase);

private:
  // configure axis rect:
  QCPColorScale *mColorScale{nullptr};
  QCPColorMap *mColorMap{nullptr};
  Configuration &mConfig;
};
