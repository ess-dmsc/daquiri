#include <core/ConsumerFactory.h>

#include <consumers/consumers_autoreg.h>

#include <consumers/StatsScalar.h>

#include <consumers/histogram_1d.h>
#include <consumers/Prebinned1D.h>
#include <consumers/TOF1D.h>
#include <consumers/TOF1DCorrelate.h>
#include <consumers/TimeDomain.h>
#include <consumers/TimeDelta1D.h>

#include <consumers/TOFVal2D.h>
#include <consumers/TOFVal2DCorrelate.h>
#include <consumers/Histogram2D.h>
#include <consumers/Image2D.h>
#include <consumers/TimeSpectrum.h>

#include <consumers/Histogram3D.h>

using namespace DAQuiri;

void consumers_autoreg()
{
  DAQUIRI_REGISTER_CONSUMER(StatsScalar)

  DAQUIRI_REGISTER_CONSUMER(Histogram1D)
  DAQUIRI_REGISTER_CONSUMER(Prebinned1D)
  DAQUIRI_REGISTER_CONSUMER(TOF1D)
  DAQUIRI_REGISTER_CONSUMER(TOF1DCorrelate)
  DAQUIRI_REGISTER_CONSUMER(TimeDomain)
  DAQUIRI_REGISTER_CONSUMER(TimeDelta1D)

  DAQUIRI_REGISTER_CONSUMER(Histogram2D)
  DAQUIRI_REGISTER_CONSUMER(Image2D)
  DAQUIRI_REGISTER_CONSUMER(TOFVal2D)
  DAQUIRI_REGISTER_CONSUMER(TOFVal2DCorrelate)
  DAQUIRI_REGISTER_CONSUMER(TimeSpectrum)

  DAQUIRI_REGISTER_CONSUMER(Histogram3D)
}
