#include <core/ConsumerFactory.h>

#include <consumers/consumers_autoreg.h>

#include <consumers/stats_scalar.h>

#include <consumers/histogram_1d.h>
#include <consumers/prebinned_1d.h>
#include <consumers/TOF1D.h>
#include <consumers/tof_1d_correlate.h>
#include <consumers/time_domain.h>
#include <consumers/time_delta_1d.h>

#include <consumers/TOFVal2D.h>
#include <consumers/TOFVal2DCorrelate.h>
#include <consumers/histogram_2d.h>
#include <consumers/image_2d.h>
#include <consumers/spectrum_time.h>

#include <consumers/histogram_3d.h>

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
