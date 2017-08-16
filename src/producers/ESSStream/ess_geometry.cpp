#include "ess_geometry.h"
//#include "custom_logger.h"

void GeometryInterpreter::add_dimension(std::string name, size_t size)
{
  names_.push_back(name);
  if (bounds_.empty())
    bounds_.push_front(1);
  bounds_.push_front(size * bounds_.front());
}

EventModel GeometryInterpreter::model(const TimeBase& tb)
{
  EventModel ret;
  ret.timebase = tb;
  for (auto n : names_)
    ret.add_value(n, 16);
  if (bounds_.size())
    bounds_.pop_front();
}

void GeometryInterpreter::interpret_id(Event& e, size_t val)
{
  size_t i = 0;
  for (auto b : bounds_)
  {
    e.set_value(i++, val / b);
    val = val % b;
  }
}

