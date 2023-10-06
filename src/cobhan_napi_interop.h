#ifndef COBHAN_NAPI_INTEROP_H
#define COBHAN_NAPI_INTEROP_H
#include "cobhan.h"
#include "hints.h"
#include "logging.h"
#include <napi.h>
#include <string>

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
nstring_utf8_byte_length(Napi::Env &env, Napi::String &str) {
  napi_status status;
  size_t utf8_length;

  status = napi_get_value_string_utf8(env, str, nullptr, 0, &utf8_length);
  if (unlikely(status != napi_ok)) {
    error_log("nstring_utf8_length",
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
    error_log("copy_nstring_to_cbuffer", "Invalid cobhan buffer byte length");
    return nullptr;
  }

  if (cobhan_buffer_size_bytes < str_utf8_byte_length) {
    error_log("copy_nstring_to_cbuffer", "String too large for cobhan buffer");
    return nullptr;
  }

  if (unlikely(verbose_flag)) {
    debug_log("copy_nstring_to_cbuffer",
              "Copying " + std::to_string(str_utf8_byte_length) + " bytes to " +
                  format_ptr(cbuffer_data_ptr(cobhan_buffer)) + " - " +
                  format_ptr((cbuffer_data_ptr(cobhan_buffer) +
                              str_utf8_byte_length)));
  }

  napi_status status;
  size_t copied_bytes;
  // NOTE: This implementation relies on the additional byte that is reserved
  // upon allocation for a NULL delimiter as methods like
  // napi_get_value_string_utf8 append a NULL delimiter
  status = napi_get_value_string_utf8(env, str, cbuffer_data_ptr(cobhan_buffer),
                                      str_utf8_byte_length + 1, &copied_bytes);
  if (unlikely(status != napi_ok)) {
    error_log("copy_nstring_to_cbuffer",
              "Napi utf8 string conversion failure: " +
                  napi_status_to_string(status));
    return nullptr;
  }

  if (unlikely(copied_bytes != str_utf8_byte_length)) {
    error_log("copy_nstring_to_cbuffer",
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

__attribute__((always_inline)) inline Napi::String
cbuffer_to_nstring(Napi::Env &env, char *cobhan_buffer) {
  napi_value output;

  int32_t cobhan_buffer_size_bytes = cbuffer_byte_length(cobhan_buffer);
  if (cobhan_buffer_size_bytes <= 0) {
    log_error_and_throw("cbuffer_to_nstring",
                        "Invalid cobhan buffer byte length");
  }

  // Using C function because it allows length delimited input
  napi_status status = napi_create_string_utf8(
      env, cbuffer_data_ptr(cobhan_buffer), cobhan_buffer_size_bytes, &output);

  if (unlikely(status != napi_ok)) {
    log_error_and_throw("cbuffer_to_nstring",
                        "napi_create_string_utf8 failed: " +
                            napi_status_to_string(status));
  }

  return Napi::String(env, output);
}

__attribute__((always_inline)) inline Napi::Buffer<unsigned char>
cbuffer_to_nbuffer(Napi::Env &env, char *cobhan_buffer) {
  int32_t cobhan_buffer_byte_length = cbuffer_byte_length(cobhan_buffer);
  if (unlikely(cobhan_buffer_byte_length <= 0)) {
    log_error_and_throw("cbuffer_to_nbuffer",
                        "Invalid cobhan buffer byte length");
  }

  if (unlikely(verbose_flag)) {
    debug_log("cbuffer_to_nbuffer",
              "cbuffer_byte_length: " +
                  std::to_string(cobhan_buffer_byte_length));
  }

  if (unlikely(cobhan_buffer_byte_length <= 0)) {
    log_error_and_throw("cbuffer_to_nbuffer",
                        "Invalid cobhan buffer byte length");
  }

  Napi::Buffer<unsigned char> nbuffer = Napi::Buffer<unsigned char>::Copy(
      env, (const unsigned char *)cbuffer_data_ptr(cobhan_buffer),
      cobhan_buffer_byte_length);

  if (unlikely(verbose_flag)) {
    debug_log("cbuffer_to_nbuffer",
              "nbuffer.ByteLength(): " + std::to_string(nbuffer.ByteLength()));
  }

  return nbuffer;
}

__attribute__((always_inline)) inline char *
copy_nbuffer_to_cbuffer(Napi::Env &env, Napi::Buffer<unsigned char> &nbuffer,
                        char *cobhan_buffer) {

  int32_t cobhan_buffer_size_bytes = cbuffer_byte_length(cobhan_buffer);
  if (unlikely(cobhan_buffer_size_bytes <= 0)) {
    error_log("copy_nbuffer_to_cbuffer", "Invalid cobhan buffer byte length");
    return nullptr;
  }

  size_t nbuffer_byte_length = nbuffer.ByteLength();
  if (nbuffer_byte_length > INT32_MAX ||
      cobhan_buffer_size_bytes < (int32_t)nbuffer_byte_length) {
    error_log("copy_nbuffer_to_cbuffer", "Buffer too large for cobhan buffer");
    return nullptr;
  }
  memcpy(cbuffer_data_ptr(cobhan_buffer), nbuffer.Data(), nbuffer_byte_length);
  configure_cbuffer(cobhan_buffer, nbuffer_byte_length);
  return cobhan_buffer;
}

// These are macros due to the use of alloca()

#define NAPI_STRING_TO_CBUFFER(env, napi_string, cobhan_buffer, bytes_copied,  \
                               max_stack_alloc_size, function_name)            \
  std::unique_ptr<char[]> napi_string##_unique_ptr;                            \
  do {                                                                         \
    /* Determine size */                                                       \
    size_t napi_string##_utf8_byte_length =                                    \
        nstring_utf8_byte_length(env, napi_string);                            \
    if (unlikely(napi_string##_utf8_byte_length == (size_t)(-1))) {            \
      log_error_and_throw(function_name,                                       \
                          "Failed to get " #napi_string " utf8 length");       \
    }                                                                          \
    if (unlikely(napi_string##_utf8_byte_length == 0)) {                       \
      log_error_and_throw(function_name, #napi_string " is empty");            \
    }                                                                          \
    /* Allocate */                                                             \
    ALLOCATE_CBUFFER_UNIQUE_PTR(cobhan_buffer, napi_string##_utf8_byte_length, \
                                napi_string##_unique_ptr,                      \
                                max_stack_alloc_size, function_name);          \
    /* Copy */                                                                 \
    cobhan_buffer = copy_nstring_to_cbuffer(env, napi_string,                  \
                                            napi_string##_utf8_byte_length,    \
                                            cobhan_buffer, &bytes_copied);     \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(function_name,                                       \
                          "Failed to copy " #napi_string " to cobhan buffer"); \
    }                                                                          \
  } while (0);

#define NAPI_BUFFER_TO_CBUFFER(env, napi_buffer, cobhan_buffer, bytes_copied,  \
                               max_stack_alloc_size, function_name)            \
  std::unique_ptr<char[]> napi_buffer##_unique_ptr;                            \
  do {                                                                         \
    /* Determine size */                                                       \
    size_t napi_buffer##_byte_length = napi_buffer.ByteLength();               \
    if (unlikely(napi_buffer##_byte_length == 0)) {                            \
      log_error_and_throw(function_name, #napi_buffer " is empty");            \
    }                                                                          \
    /* Allocate */                                                             \
    ALLOCATE_CBUFFER_UNIQUE_PTR(cobhan_buffer, napi_buffer##_byte_length,      \
                                napi_buffer##_unique_ptr,                      \
                                max_stack_alloc_size, function_name);          \
    /* Copy */                                                                 \
    cobhan_buffer = copy_nbuffer_to_cbuffer(env, napi_buffer, cobhan_buffer);  \
    if (unlikely(cobhan_buffer == nullptr)) {                                  \
      log_error_and_throw(function_name,                                       \
                          "Failed to copy " #napi_buffer " to cobhan buffer"); \
    }                                                                          \
    bytes_copied = napi_buffer##_byte_length;                                  \
  } while (0);

#endif
