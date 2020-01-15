#include <core/util/logger.h>
#include <producers/ESSStream/fb_parser.h>

fb_parser::fb_parser() : Producer() {}

void fb_parser::boot() {
  if (!(status_ & ProducerStatus::can_boot))
    return;
  status_ = ProducerStatus::loaded | ProducerStatus::booted;
}

void fb_parser::die() {
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}
