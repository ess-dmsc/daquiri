#include "fb_parser.h"

#include "custom_logger.h"

void fb_parser::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
    return;
  status_ = ProducerStatus::loaded | ProducerStatus::booted;
}

void fb_parser::die()
{
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}
