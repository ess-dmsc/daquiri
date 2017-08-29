#pragma once

#include "consumer_factory.h"

#include "histogram_1d.h"
DAQUIRI_REGISTER_CONSUMER(Histogram1D)

#include "coincidence_1d.h"
DAQUIRI_REGISTER_CONSUMER(Coincidence1D)

#include "coincidence_2d.h"
DAQUIRI_REGISTER_CONSUMER(Coincidence2D)

#include "image_2d.h"
DAQUIRI_REGISTER_CONSUMER(Image2D)

#include "tof_1d.h"
DAQUIRI_REGISTER_CONSUMER(TOF1D)
