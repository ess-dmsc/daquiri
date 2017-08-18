#include "ess_geometry.h"
//#include "custom_logger.h"

void GeometryInterpreter::add_dimension(std::string name, size_t size)
{
  if (!size)
    return;
  names_.push_back(name);
  dimensions_[name] = size;
  if (coefs_.empty())
    coefs_.push_front(1);
  coefs_.push_front(size * coefs_.front());
}

EventModel GeometryInterpreter::model(const TimeBase& tb) const
{
  EventModel ret;
  ret.timebase = tb;
  for (auto n : names_)
    ret.add_value(n, 16);
  return ret;
}

void GeometryInterpreter::interpret_id(Event& e, size_t val) const
{
  if (coefs_.empty() || !val)
    return;
  val--;
  size_t i = 0;
  auto b = coefs_.begin(); b++;
  while (b != coefs_.end())
  {
    e.set_value(i++, val / (*b));
    val = val % (*b);
    b++;
  }
}

