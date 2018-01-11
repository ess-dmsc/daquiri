#include "consumer_factory.h"
#include "consumers_autoreg.h"

#include "histogram_1d.h"
#include "prebinned_1d.h"
#include "tof_1d.h"
#include "tof_1d_correlate.h"
#include "tof_val_2d.h"
#include "time_domain.h"
#include "histogram_2d.h"
#include "image_2d.h"
#include "spectrum_time.h"

void consumers_autoreg()
{
  DAQUIRI_REGISTER_CONSUMER(Histogram1D)
  DAQUIRI_REGISTER_CONSUMER(Prebinned1D)
  DAQUIRI_REGISTER_CONSUMER(TOF1D)
  DAQUIRI_REGISTER_CONSUMER(TOF1DCorrelate)
  DAQUIRI_REGISTER_CONSUMER(TOFVal2D)
  DAQUIRI_REGISTER_CONSUMER(TimeDomain)
  DAQUIRI_REGISTER_CONSUMER(Histogram2D)
  DAQUIRI_REGISTER_CONSUMER(Image2D)
  DAQUIRI_REGISTER_CONSUMER(TimeSpectrum)
}
