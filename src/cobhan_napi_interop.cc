#include <stddef.h>
size_t est_intermediate_key_overhead = 0;
size_t safety_padding_bytes = 0;

void set_est_intermediate_key_overhead(size_t est_intermediate_key_overhead) {
  ::est_intermediate_key_overhead = est_intermediate_key_overhead;
}

void set_safety_padding_bytes(size_t safety_padding_bytes) {
  ::safety_padding_bytes = safety_padding_bytes;
}
