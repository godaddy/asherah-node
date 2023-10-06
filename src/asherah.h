#ifndef ASHERAH_H
#define ASHERAH_H
#include <cstdint>
#include <napi.h>
#include <stddef.h>

size_t estimate_asherah_output_size_bytes(size_t data_byte_len,
                                          size_t partition_byte_len);

const char *asherah_cobhan_error_to_string(int32_t error);

Napi::String encrypt_to_json(Napi::Env &env, size_t partition_bytes,
                             size_t data_bytes,
                             char *partition_id_cobhan_buffer,
                             char *input_cobhan_buffer);

#endif
