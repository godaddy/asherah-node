#ifndef COBHAN_H
#define COBHAN_H
#include "logging.h"
#include <cstdint>
#include <stddef.h>
#include <string>

extern size_t safety_padding_bytes;

const size_t cobhan_header_size_bytes = 64 / 8;
const size_t canary_size = sizeof(int32_t) * 2;
const int32_t canary_constant = 0xdeadbeef;

__attribute__((always_inline)) inline void
set_safety_padding_bytes(size_t new_safety_padding_bytes) {
  safety_padding_bytes = new_safety_padding_bytes;
}

__attribute__((always_inline)) inline int32_t
cbuffer_byte_length(char *cobhan_buffer) {
  return *((int32_t *)cobhan_buffer);
}

__attribute__((always_inline)) inline char *
cbuffer_data_ptr(char *cobhan_buffer) {
  return cobhan_buffer + cobhan_header_size_bytes;
}

__attribute__((always_inline)) inline size_t
calculate_cobhan_buffer_allocation_size(size_t data_len_bytes) {
  return data_len_bytes + cobhan_header_size_bytes +
         1 + // Add one for possible NULL delimiter due to Node string functions
         canary_size             // Add space for canary value
         + safety_padding_bytes; // Add safety padding if configured
}

__attribute__((always_inline)) inline void
configure_cbuffer(char *cobhan_buffer, size_t length) {
  if (unlikely(verbose_flag)) {
    debug_log("configure_cbuffer", "configure_cbuffer(" +
                                       format_ptr(cobhan_buffer) + ", " +
                                       std::to_string(length) + ")");
  }

  *((int32_t *)cobhan_buffer) = length;
  // Reserved for future use
  *((int32_t *)(cobhan_buffer + sizeof(int32_t))) = 0;

  // Write canary values
  char *data_ptr = cbuffer_data_ptr(cobhan_buffer);
#ifdef LOG_CANARY_WRITES
  if (unlikely(verbose_flag)) {
    debug_log("configure_cbuffer",
              "Writing first canary at " + format_ptr(data_ptr + length + 1));
  }
#endif

  // First canary value is a int32_t 0 which gives us four NULLs
  *((int32_t *)(data_ptr + length + 1)) = 0;
#ifdef LOG_CANARY_WRITES
  if (unlikely(verbose_flag)) {
    debug_log("configure_cbuffer",
              "Writing second canary at " +
                  format_ptr(data_ptr + length + 1 + sizeof(int32_t)));
  }
#endif

  // Second canary value is a int32_t 0xdeadbeef
  *((int32_t *)(data_ptr + length + 1 + sizeof(int32_t))) = canary_constant;
}

__attribute__((always_inline)) inline char *
get_canary_ptr(char *cobhan_buffer) {
  int32_t cobhan_buffer_size_bytes = cbuffer_byte_length(cobhan_buffer);
  return cbuffer_data_ptr(cobhan_buffer) + cobhan_buffer_size_bytes + 1;
}

__attribute__((always_inline)) inline bool check_canary_ptr(char *canary_ptr) {
  int32_t zero_value = *((int32_t *)(canary_ptr));
  if (zero_value != 0) {
    std::string error_msg =
        "Canary check failed: " + std::to_string(zero_value) + " != 0";
    error_log("canary_check_cbuffer", error_msg);
    return false;
  }
  int32_t canary_value = *((int32_t *)(canary_ptr + sizeof(int32_t)));
  if (canary_value != canary_constant) {
    std::string error_msg =
        "Canary check failed: " + std::to_string(canary_value) +
        " != " + std::to_string(canary_constant);
    error_log("canary_check_cbuffer", error_msg);
    return false;
  }
  return true;
}

__attribute__((always_inline)) inline std::unique_ptr<char[]>
heap_allocate_cbuffer(const char *variable_name, size_t size_bytes) {
  size_t cobhan_buffer_allocation_size =
      calculate_cobhan_buffer_allocation_size(size_bytes);
  if (unlikely(verbose_flag)) {
    std::string log_msg =
        "heap_allocate_cbuffer(" + std::to_string(size_bytes) +
        ") (heap) cobhan_buffer_allocation_size: " +
        std::to_string(cobhan_buffer_allocation_size) + " for " + variable_name;
    debug_log("allocate_cbuffer", log_msg);
  }

  char *cobhan_buffer = new (std::nothrow) char[cobhan_buffer_allocation_size];
  if (unlikely(cobhan_buffer == nullptr)) {
    std::string error_msg = "new[" +
                            std::to_string(cobhan_buffer_allocation_size) +
                            "] returned null";
    error_log("allocate_cbuffer", error_msg);
    return nullptr;
  }
  std::unique_ptr<char[]> cobhan_buffer_unique_ptr(cobhan_buffer);
  configure_cbuffer(cobhan_buffer, size_bytes + safety_padding_bytes);
  return cobhan_buffer_unique_ptr;
}

#define ALLOCATE_CBUFFER_UNIQUE_PTR(cobhan_buffer, buffer_size, unique_ptr,    \
                                    max_stack_alloc_size, function_name)       \
  do {                                                                         \
    if (buffer_size < max_stack_alloc_size) {                                  \
      /* If the buffer is small enough, allocate it on the stack */            \
      size_t cobhan_buffer##_cobhan_buffer_allocation_size =                   \
          calculate_cobhan_buffer_allocation_size(buffer_size);                \
      debug_log_alloca(function_name, #cobhan_buffer,                          \
                       cobhan_buffer##_cobhan_buffer_allocation_size);         \
      cobhan_buffer =                                                          \
          (char *)alloca(cobhan_buffer##_cobhan_buffer_allocation_size);       \
      configure_cbuffer(cobhan_buffer, buffer_size);                           \
    } else {                                                                   \
      /* Otherwise, allocate it on the heap */                                 \
      unique_ptr = heap_allocate_cbuffer(#cobhan_buffer, buffer_size);         \
      cobhan_buffer = unique_ptr.get();                                        \
    }                                                                          \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(function_name,                                       \
                          "Failed to allocate " #cobhan_buffer);               \
    }                                                                          \
  } while (0);

#define ALLOCATE_CBUFFER(cobhan_buffer, buffer_size, max_stack_alloc_size,     \
                         function_name)                                        \
  std::unique_ptr<char[]> cobhan_buffer##_unique_ptr;                          \
  ALLOCATE_CBUFFER_UNIQUE_PTR(cobhan_buffer, buffer_size,                      \
                              cobhan_buffer##_unique_ptr,                      \
                              max_stack_alloc_size, function_name);

#endif
