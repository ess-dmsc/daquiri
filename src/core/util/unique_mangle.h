#pragma once

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define UNIQUE_MANGLE(base) CONCAT(base, __LINE__)
