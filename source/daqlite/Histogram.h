
#pragma once

class Histogram {
public:
  Histogram(int x, int y, int z);
private:
  uint32_t * buffer{nullptr};
}
