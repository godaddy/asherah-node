#ifndef COBHAN_NAPI_INTEROP_H
#define COBHAN_NAPI_INTEROP_H
#include "hints.h"
#include "logging.h"
#include <napi.h>
#include <string>

// Stupid hack to get around extern issues
size_t *get_est_intermediate_key_overhead_ptr();
size_t *get_safety_padding_bytes_ptr();

size_t *est_intermediate_key_overhead_ptr =
    get_est_intermediate_key_overhead_ptr();
size_t *safety_padding_bytes_ptr = get_safety_padding_bytes_ptr();

const size_t est_encryption_overhead = 48;
const size_t est_envelope_overhead = 185;
const double base64_overhead = 1.34;

const size_t cobhan_header_size_bytes = 64 / 8;
const size_t canary_size = sizeof(int32_t) * 2;
const int32_t canary_constant = 0xdeadbeef;

std::string napi_status_to_string(napi_status status) {
  switch (status) {
  case napi_ok:
    return "napi_ok";
  case napi_invalid_arg:
    return "napi_invalid_arg";
  case napi_object_expected:
    return "napi_object_expected";
  case napi_string_expected:
    return "napi_string_expected";
  case napi_name_expected:
    return "napi_name_expected";
  case napi_function_expected:
    return "napi_function_expected";
  case napi_number_expected:
    return "napi_number_expected";
  case napi_boolean_expected:
    return "napi_boolean_expected";
  case napi_array_expected:
    return "napi_array_expected";
  case napi_generic_failure:
    return "napi_generic_failure";
  case napi_pending_exception:
    return "napi_pending_exception";
  case napi_cancelled:
    return "napi_cancelled";
  case napi_escape_called_twice:
    return "napi_escape_called_twice";
  case napi_handle_scope_mismatch:
    return "napi_handle_scope_mismatch";
  case napi_callback_scope_mismatch:
    return "napi_callback_scope_mismatch";
  case napi_queue_full:
    return "napi_queue_full";
  case napi_closing:
    return "napi_closing";
  case napi_bigint_expected:
    return "napi_bigint_expected";
  case napi_date_expected:
    return "napi_date_expected";
  case napi_arraybuffer_expected:
    return "napi_arraybuffer_expected";
  case napi_detachable_arraybuffer_expected:
    return "napi_detachable_arraybuffer_expected";
  case napi_would_deadlock:
    return "napi_would_deadlock";
  default:
    return "Unknown napi_status";
  }
}

__attribute__((always_inline)) inline int32_t
cbuffer_byte_length(char *cobhan_buffer) {
  return *((int32_t *)cobhan_buffer);
}

__attribute__((always_inline)) inline void
log_error_and_throw(Napi::Env &env, const char *function_name,
                    std::string error_msg) {
  error_log(function_name, error_msg);
  Napi::Error::New(env, function_name + (": " + error_msg))
      .ThrowAsJavaScriptException();
}

__attribute__((always_inline)) inline size_t
calculate_cobhan_buffer_allocation_size(size_t data_len_bytes) {
  return data_len_bytes + cobhan_header_size_bytes +
         1 + // Add one for possible NULL delimiter due to Node string functions
         canary_size                  // Add space for canary value
         + *safety_padding_bytes_ptr; // Add safety padding if configured
}

__attribute__((always_inline)) inline size_t
estimate_asherah_output_size_bytes(size_t data_byte_len,
                                   size_t partition_byte_len) {
  // Add one rather than using std::ceil to round up
  double est_data_byte_len =
      (double(data_byte_len + est_encryption_overhead) * base64_overhead) + 1;

  size_t asherah_output_size_bytes = size_t(
      est_envelope_overhead + *est_intermediate_key_overhead_ptr +
      partition_byte_len + est_data_byte_len + *safety_padding_bytes_ptr);
  if (unlikely(verbose_flag)) {
    std::string log_msg =
        "estimate_asherah_output_size(" + std::to_string(data_byte_len) + ", " +
        std::to_string(partition_byte_len) +
        ") est_data_byte_len: " + std::to_string(est_data_byte_len) +
        " asherah_output_size_bytes: " +
        std::to_string(asherah_output_size_bytes);
    debug_log("estimate_asherah_output_size", log_msg);
  }
  return asherah_output_size_bytes;
}

__attribute__((always_inline)) inline char *
cbuffer_data_ptr(char *cobhan_buffer) {
  return cobhan_buffer + cobhan_header_size_bytes;
}

__attribute__((always_inline)) inline void configure_cbuffer(char *buffer,
                                                             size_t length) {
  if (verbose_flag) {
    debug_log("configure_cbuffer", "configure_cbuffer(" +
                                       std::to_string((intptr_t)buffer) + ", " +
                                       std::to_string(length) + ")");
  }

  *((int32_t *)buffer) = length;
  // Reserved for future use
  *((int32_t *)(buffer + sizeof(int32_t))) = 0;

  // Write canary values
  char *data_ptr = cbuffer_data_ptr(buffer);
  if (verbose_flag) {
    debug_log("configure_cbuffer",
              "Writing first canary at " +
                  std::to_string((intptr_t)(data_ptr + length + 1)));
  }

  // First canary value is a int32_t 0 which gives us four NULLs
  *((int32_t *)(data_ptr + length + 1)) = 0;

  if (verbose_flag) {
    debug_log("configure_cbuffer",
              "Writing second canary at " +
                  std::to_string(
                      (intptr_t)(data_ptr + length + 1 + sizeof(int32_t))));
  }

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
  configure_cbuffer(cobhan_buffer, size_bytes + *safety_padding_bytes_ptr);
  return cobhan_buffer_unique_ptr;
}

__attribute__((always_inline)) inline Napi::String
cbuffer_to_nstring(Napi::Env &env, char *cobhan_buffer) {
  napi_value output;

  int32_t cobhan_buffer_size_bytes = cbuffer_byte_length(cobhan_buffer);
  if (cobhan_buffer_size_bytes <= 0) {
    log_error_and_throw(env, "cbuffer_to_nstring",
                        "Invalid cobhan buffer byte length");
  }

  // Using C function because it allows length delimited input
  napi_status status = napi_create_string_utf8(
      env, cbuffer_data_ptr(cobhan_buffer),
      cobhan_buffer_size_bytes, &output);

  if (unlikely(status != napi_ok)) {
    log_error_and_throw(env, "cbuffer_to_nstring",
                        "napi_create_string_utf8 failed: " +
                            napi_status_to_string(status));
  }

  return Napi::String(env, output);
}

__attribute__((always_inline)) inline size_t
nstring_utf8_byte_length(Napi::Env &env, Napi::String &str) {
  napi_status status;
  size_t utf8_length;

  status = napi_get_value_string_utf8(env, str, nullptr, 0, &utf8_length);
  if (unlikely(status != napi_ok)) {
    log_error_and_throw(env, "nstring_utf8_length",
                        "napi_get_value_string_utf8 length check failed: " +
                            napi_status_to_string(status));
    return (size_t)(-1);
  }

  return utf8_length;
}

__attribute__((always_inline)) inline char *
copy_nstring_to_cbuffer(Napi::Env &env, Napi::String &str,
                        size_t str_utf8_byte_length, char *cobhan_buffer,
                        size_t *byte_length = nullptr) {

  size_t cobhan_buffer_size_bytes = cbuffer_byte_length(cobhan_buffer);
  if (unlikely(cobhan_buffer_size_bytes <= 0)) {
    log_error_and_throw(env, "copy_nstring_to_cbuffer",
                        "Invalid cobhan buffer byte length");
    return nullptr;
  }

  if (cobhan_buffer_size_bytes < str_utf8_byte_length) {
    log_error_and_throw(env, "copy_nstring_to_cbuffer",
                        "String too large for cobhan buffer");
    return nullptr;
  }

  if (unlikely(verbose_flag)) {
    debug_log(
        "copy_nstring_to_cbuffer",
        "Copying " + std::to_string(str_utf8_byte_length) + " bytes to " +
            std::to_string(
                (intptr_t)cbuffer_data_ptr(cobhan_buffer)) +
            "-" +
            std::to_string((intptr_t)(cbuffer_data_ptr(cobhan_buffer) +
                                      str_utf8_byte_length)));
  }

  napi_status status;
  size_t copied_bytes;
  // NOTE: This implementation relies on the additional byte that is reserved
  // upon allocation for a NULL delimiter as methods like
  // napi_get_value_string_utf8 append a NULL delimiter
  status = napi_get_value_string_utf8(env, str,
                                      cbuffer_data_ptr(cobhan_buffer),
                                      str_utf8_byte_length + 1, &copied_bytes);
  if (unlikely(status != napi_ok)) {
    log_error_and_throw(env, "copy_nstring_to_cbuffer",
                        "Napi utf8 string conversion failure: " +
                            napi_status_to_string(status));
    return nullptr;
  }

  if (unlikely(copied_bytes != str_utf8_byte_length)) {
    log_error_and_throw(env, "copy_nstring_to_cbuffer",
                        "Did not copy expected number of bytes " +
                            std::to_string(str_utf8_byte_length) + " copied " +
                            std::to_string(copied_bytes));
    return nullptr;
  }

  configure_cbuffer(cobhan_buffer, copied_bytes);

  if (byte_length != nullptr)
    *byte_length = copied_bytes;
  return cobhan_buffer;
}

__attribute__((always_inline)) inline char *
copy_nbuffer_to_cbuffer(Napi::Env &env, Napi::Buffer<unsigned char> &nbuffer,
                        char *cobhan_buffer) {

  int32_t cobhan_buffer_size_bytes = cbuffer_byte_length(cobhan_buffer);
  if (unlikely(cobhan_buffer_size_bytes <= 0)) {
    log_error_and_throw(env, "copy_nbuffer_to_cbuffer",
                        "Invalid cobhan buffer byte length");
    return nullptr;
  }

  size_t nbuffer_byte_length = nbuffer.ByteLength();
  if (nbuffer_byte_length > INT32_MAX ||
      cobhan_buffer_size_bytes < (int32_t)nbuffer_byte_length) {
    log_error_and_throw(env, "copy_nbuffer_to_cbuffer",
                        "Buffer too large for cobhan buffer");
    return nullptr;
  }
  memcpy(cbuffer_data_ptr(cobhan_buffer), nbuffer.Data(),
         nbuffer_byte_length);
  configure_cbuffer(cobhan_buffer, nbuffer_byte_length);
  return cobhan_buffer;
}

__attribute__((always_inline)) inline Napi::Buffer<unsigned char>
cbuffer_to_nbuffer(Napi::Env &env, char *cobhan_buffer) {
  int32_t cobhan_buffer_byte_length = cbuffer_byte_length(cobhan_buffer);
  if (unlikely(cobhan_buffer_byte_length <= 0)) {
    log_error_and_throw(env, "cbuffer_to_nbuffer",
                        "Invalid cobhan buffer byte length");
  }

  if (unlikely(verbose_flag)) {
    debug_log("cbuffer_to_nbuffer",
              "cbuffer_byte_length: " +
                  std::to_string(cobhan_buffer_byte_length));
  }

  if (unlikely(cobhan_buffer_byte_length <= 0)) {
    log_error_and_throw(env, "cbuffer_to_nbuffer",
                        "Invalid cobhan buffer byte length");
  }

  Napi::Buffer<unsigned char> nbuffer = Napi::Buffer<unsigned char>::Copy(
      env, (const unsigned char*)cbuffer_data_ptr(cobhan_buffer),
      cobhan_buffer_byte_length);

  if (unlikely(verbose_flag)) {
    debug_log("cbuffer_to_nbuffer",
              "nbuffer.ByteLength(): " + std::to_string(nbuffer.ByteLength()));
  }

  return nbuffer;
}

// These are macros due to the use of alloca()

#define NAPI_STRING_TO_CBUFFER(napi_string, cobhan_buffer, bytes_copied,       \
                               function_name)                                  \
  std::unique_ptr<char[]> napi_string##_cobhan_buffer_unique_ptr;              \
  do {                                                                         \
    /* Determine size */                                                       \
    size_t napi_string##_utf8_byte_length;                                     \
    napi_string##_utf8_byte_length =                                           \
        nstring_utf8_byte_length(env, napi_string);                            \
    if (unlikely(napi_string##_utf8_byte_length == (size_t)(-1))) {            \
      log_error_and_throw(env, function_name,                                  \
                          "Failed to get " #napi_string " utf8 length");       \
    }                                                                          \
    if (unlikely(napi_string##_utf8_byte_length == 0)) {                       \
      log_error_and_throw(env, function_name, #napi_string " is empty");       \
    }                                                                          \
    /* Allocate */                                                             \
    if (napi_string##_utf8_byte_length < max_stack_alloc_size) {               \
      /* If the buffer is small enough, allocate it on the stack  */           \
      size_t napi_string##_cobhan_buffer_allocation_size =                     \
          calculate_cobhan_buffer_allocation_size(                             \
              napi_string##_utf8_byte_length);                                 \
      debug_log_alloca(function_name, #napi_string "_cobhan_buffer",           \
                       napi_string##_cobhan_buffer_allocation_size);           \
      cobhan_buffer =                                                          \
          (char *)alloca(napi_string##_cobhan_buffer_allocation_size);         \
      configure_cbuffer(cobhan_buffer, napi_string##_utf8_byte_length);        \
    } else {                                                                   \
      /* Otherwise, allocate it on the heap */                                 \
      napi_string##_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(          \
          cobhan_buffer, napi_string##_utf8_byte_length);                      \
      cobhan_buffer = napi_string##_cobhan_buffer_unique_ptr.get();            \
    }                                                                          \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(env, function_name,                                  \
                          "Failed to allocate " #napi_string                   \
                          " cobhan buffer");                                   \
    }                                                                          \
    /* Copy */                                                                 \
    cobhan_buffer = copy_nstring_to_cbuffer(env, napi_string,                  \
                                            napi_string##_utf8_byte_length,    \
                                            cobhan_buffer, &bytes_copied);     \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(env, function_name,                                  \
                          "Failed to copy " #napi_string " to cobhan buffer"); \
    }                                                                          \
  } while (0);

#define NAPI_BUFFER_TO_CBUFFER(napi_buffer, cobhan_buffer, bytes_copied,       \
                               function_name)                                  \
  std::unique_ptr<char[]> napi_buffer##_unique_ptr;                            \
  do {                                                                         \
    /* Determine size */                                                       \
    size_t napi_buffer##_byte_length = napi_buffer.ByteLength();               \
    if (unlikely(napi_buffer##_byte_length == 0)) {                            \
      log_error_and_throw(env, function_name, #napi_buffer " is empty");       \
    }                                                                          \
    /* Allocate */                                                             \
    if (napi_buffer##_byte_length < max_stack_alloc_size) {                    \
      /* If the buffer is small enough, allocate it on the stack */            \
      size_t napi_buffer##_cobhan_buffer_allocation_size =                     \
          calculate_cobhan_buffer_allocation_size(napi_buffer##_byte_length);  \
      debug_log_alloca(function_name, #cobhan_buffer,                          \
                       napi_buffer##_cobhan_buffer_allocation_size);           \
      cobhan_buffer =                                                          \
          (char *)alloca(napi_buffer##_cobhan_buffer_allocation_size);         \
      configure_cbuffer(cobhan_buffer, napi_buffer##_byte_length);             \
    } else {                                                                   \
      /* Otherwise, allocate it on the heap */                                 \
      napi_buffer##_unique_ptr =                                               \
          heap_allocate_cbuffer(#cobhan_buffer, napi_buffer##_byte_length);    \
      cobhan_buffer = napi_buffer##_unique_ptr.get();                          \
    }                                                                          \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(                                                     \
          env, function_name,                                                  \
          "Failed to allocate cobhan buffer for " #napi_buffer);               \
    }                                                                          \
    /* Copy */                                                                 \
    cobhan_buffer = copy_nbuffer_to_cbuffer(env, napi_buffer, cobhan_buffer);  \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(env, function_name,                                  \
                          "Failed to copy " #napi_buffer " to cobhan buffer"); \
    }                                                                          \
    bytes_copied = napi_buffer##_byte_length;                                  \
  } while (0);

#define ALLOCATE_OUTPUT_CBUFFER(cobhan_buffer, buffer_size, function_name)     \
  std::unique_ptr<char[]> cobhan_buffer##_unique_ptr;                          \
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
      cobhan_buffer##_unique_ptr =                                             \
          heap_allocate_cbuffer(#cobhan_buffer, buffer_size);                  \
      cobhan_buffer = output_cobhan_buffer_unique_ptr.get();                   \
    }                                                                          \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(env, function_name,                                  \
                          "Failed to allocate " #cobhan_buffer);               \
    }                                                                          \
  } while (0);

#endif
