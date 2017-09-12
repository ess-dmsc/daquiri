#pragma once

#include "consumer_factory.h"

#include "histogram_1d.h"
DAQUIRI_REGISTER_CONSUMER(Histogram1D)

#include "coincidence_1d.h"
DAQUIRI_REGISTER_CONSUMER(Coincidence1D)

#include "coincidence_2d.h"
DAQUIRI_REGISTER_CONSUMER(Coincidence2D)

#include "histogram_2d.h"
DAQUIRI_REGISTER_CONSUMER(Histogram2D)

#include "tof_1d.h"
DAQUIRI_REGISTER_CONSUMER(TOF1D)

#include "time_domain.h"
DAQUIRI_REGISTER_CONSUMER(TimeDomain)

#include "spectrum_time.h"
DAQUIRI_REGISTER_CONSUMER(TimeSpectrum)
