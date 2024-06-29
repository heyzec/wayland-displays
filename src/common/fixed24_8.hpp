#include <cstdint>

typedef int32_t fixed24_8;

inline float fixed_to_float(fixed24_8 n) {
  return ((float)n / 256);
}

inline fixed24_8 float_to_fixed(float n) {
  return n * 256;
}
