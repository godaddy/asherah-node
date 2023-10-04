#ifndef COBHAN_NAPI_INTEROP_H
#define COBHAN_NAPI_INTEROP_H
#include <string>
#define NODE_ADDON_API_DISABLE_DEPRECATED
#include <napi.h>
#include "logging.h"
#include "hints.h"

const size_t est_encryption_overhead = 48;
const size_t est_envelope_overhead = 185;
const double base64_overhead = 1.34;

const size_t cobhan_header_size_bytes = 64 / 8;

size_t get_est_intermediate_key_overhead();
size_t get_safety_padding_bytes();
void set_est_intermediate_key_overhead(size_t est_intermediate_key_overhead);
void set_safety_padding_bytes(size_t safety_padding_bytes);

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

__attribute__((always_inline)) inline size_t
cbuffer_byte_length(char *cobhan_buffer) {
  return *((int *)cobhan_buffer);
}

__attribute__((always_inline)) inline Napi::Value
log_error_and_throw(Napi::Env &env, const char *function_name,
                 std::string error_msg) {
  error_log(function_name, error_msg);
  Napi::Error::New(env, function_name + (": " + error_msg))
      .ThrowAsJavaScriptException();
  return env.Null();
}

__attribute__((always_inline)) inline size_t
calculate_cobhan_buffer_size_bytes(size_t data_len_bytes) {
  return data_len_bytes + cobhan_header_size_bytes + get_safety_padding_bytes() +
         1; // Add one for possible NULL delimiter due to Node string functions
}

__attribute__((always_inline)) inline size_t
estimate_asherah_output_size_bytes(size_t data_byte_len,
                                   size_t partition_byte_len) {
  // Add one rather than using std::ceil to round up
  double est_data_byte_len =
      (double(data_byte_len + est_encryption_overhead) * base64_overhead) + 1;

  size_t asherah_output_size_bytes =
      size_t(est_envelope_overhead + get_est_intermediate_key_overhead() +
             partition_byte_len + est_data_byte_len + get_safety_padding_bytes());
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

__attribute__((always_inline)) inline void configure_cbuffer(char *buffer,
                                                             size_t length) {
  *((int32_t *)buffer) = length;
  // Reserved for future use
  *((int32_t *)(buffer + sizeof(int32_t))) = 0;
}

__attribute__((always_inline)) inline std::unique_ptr<char[]>
heap_allocate_cbuffer(const char *variable_name, size_t size_bytes) {
  size_t cobhan_buffer_size_bytes =
      calculate_cobhan_buffer_size_bytes(size_bytes);
  if (unlikely(verbose_flag)) {
    std::string log_msg =
        "heap_allocate_cbuffer(" + std::to_string(size_bytes) +
        ") (heap) cobhan_buffer_size_bytes: " +
        std::to_string(cobhan_buffer_size_bytes) + " for " + variable_name;
    debug_log("allocate_cbuffer", log_msg);
  }

  char *cobhan_buffer = new (std::nothrow) char[cobhan_buffer_size_bytes];
  if (unlikely(cobhan_buffer == nullptr)) {
    std::string error_msg =
        "new[" + std::to_string(cobhan_buffer_size_bytes) + " returned null";
    error_log("allocate_cbuffer", error_msg);
    return nullptr;
  }
  std::unique_ptr<char[]> cobhan_buffer_unique_ptr(cobhan_buffer);
  configure_cbuffer(cobhan_buffer, size_bytes + get_safety_padding_bytes());
  return cobhan_buffer_unique_ptr;
}

__attribute__((always_inline)) inline Napi::Value
cbuffer_to_nstring(Napi::Env &env, char *cobhan_buffer) {
  napi_value output;

  // Using C function because it allows length delimited input
  napi_status status = napi_create_string_utf8(
      env, ((const char *)cobhan_buffer) + cobhan_header_size_bytes,
      cbuffer_byte_length(cobhan_buffer), &output);

  if (unlikely(status != napi_ok)) {
    return log_error_and_throw(env, "cbuffer_to_nstring",
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
  if (cobhan_buffer_size_bytes < str_utf8_byte_length) {
    log_error_and_throw(env, "copy_nstring_to_cbuffer",
                     "String too large for cobhan buffer");
    return nullptr;
  }

  napi_status status;
  size_t copied_bytes;
  // NOTE: This implementation relies on the additional byte that is reserved
  // upon allocation for a NULL delimiter as methods like
  // napi_get_value_string_utf8 append a NULL delimiter
  status = napi_get_value_string_utf8(
      env, str, cobhan_buffer + cobhan_header_size_bytes,
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
  size_t nbuffer_byte_length = nbuffer.ByteLength();
  if (cbuffer_byte_length(cobhan_buffer) < nbuffer_byte_length) {
    log_error_and_throw(env, "copy_nbuffer_to_cbuffer",
                     "Buffer too large for cobhan buffer");
    return nullptr;
  }
  memcpy(cobhan_buffer + cobhan_header_size_bytes, nbuffer.Data(),
         nbuffer_byte_length);
  configure_cbuffer(cobhan_buffer, nbuffer_byte_length);
  return cobhan_buffer;
}

__attribute__((always_inline)) inline Napi::Buffer<unsigned char>
cbuffer_to_nbuffer(Napi::Env &env, char *cobhan_buffer) {
  return Napi::Buffer<unsigned char>::Copy(
      env, ((unsigned char *)cobhan_buffer) + cobhan_header_size_bytes,
      cbuffer_byte_length(cobhan_buffer));
}

#endif
