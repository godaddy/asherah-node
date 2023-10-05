#include <stddef.h>
static size_t est_intermediate_key_overhead = 0;
static size_t safety_padding_bytes = 0;

size_t* get_est_intermediate_key_overhead_ptr() {
  return &est_intermediate_key_overhead;
}

size_t* get_safety_padding_bytes_ptr() {
  return &safety_padding_bytes;
}
