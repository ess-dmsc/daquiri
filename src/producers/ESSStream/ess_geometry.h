#pragma once

#include <list>
#include "event.h"

using namespace DAQuiri;

class GeometryInterpreter
{
  public:
    GeometryInterpreter() {}
    void add_dimension(std::string name, size_t size);
    EventModel model(const TimeBase& tb);
    void interpret_id(Event& e, size_t val);

  private:
    std::vector<std::string> names_;
    std::list<size_t> bounds_;
};
