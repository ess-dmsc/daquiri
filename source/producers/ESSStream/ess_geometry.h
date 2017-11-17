#pragma once

#include <list>
#include <map>
#include "event.h"

using namespace DAQuiri;

class GeometryInterpreter
{
  public:
    GeometryInterpreter() {}
    void add_dimension(std::string name, size_t size);
    EventModel model(const TimeBase& tb) const;
    void interpret_id(Event& e, size_t val) const;

    std::vector<std::string> names_;
    std::map<std::string, size_t> dimensions_;
    std::list<size_t> coefs_;
};
