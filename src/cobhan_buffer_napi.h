#ifndef COBHAN_BUFFER_NAPI_H
#define COBHAN_BUFFER_NAPI_H

#include "cobhan_buffer.h"
#include "napi_utils.h"
#include <napi.h>
#include <stdexcept>

#ifndef NAPI_CPP_EXCEPTIONS
#error Support for C++ exceptions is required
#endif

class CobhanBufferNapi : public CobhanBuffer {
public:
  // Constructor from a Napi::String
  CobhanBufferNapi(const Napi::Env &env, const Napi::String &napiString)
      : CobhanBuffer(NapiUtils::GetUtf8StringLength(env, napiString) +
                     1), env(env) { // Add one for possible NULL delimiter due to Node
                          // string functions
    copy_from_string(env, napiString);
  }

  // Constructor from Napi::Buffer<unsigned char>
  explicit CobhanBufferNapi(const Napi::Env &env,
                            const Napi::Buffer<unsigned char> &napiBuffer)
      : CobhanBuffer(napiBuffer.ByteLength()), env(env) {
    std::memcpy(get_data_ptr(), napiBuffer.Data(), napiBuffer.ByteLength());
  }

  // Constructor from Napi::Value
  explicit CobhanBufferNapi(const Napi::Env &env, const Napi::Value &napiValue)
      : CobhanBuffer(ValueToDataSize(env, napiValue)), env(env) {
    if (napiValue.IsString()) {
      copy_from_string(env, napiValue.As<Napi::String>());
    } else if (napiValue.IsBuffer()) {
      auto napiBuffer =
          napiValue.As<Napi::Buffer<unsigned char>>();
      std::memcpy(get_data_ptr(), napiBuffer.Data(), napiBuffer.Length());
    } else {
      NapiUtils::ThrowException(
          env, "Expected a Napi::String or Napi::Buffer<unsigned "
               "char> as the value.");
    }
  }

  // Constructor from a Napi::String to an externally allocated buffer
  CobhanBufferNapi(const Napi::Env &env,
                   const Napi::Buffer<unsigned char> &napiBuffer, char *cbuffer,
                   size_t allocation_size)
      : CobhanBuffer(cbuffer, allocation_size), env(env) {
    std::memcpy(get_data_ptr(), napiBuffer.Data(), napiBuffer.Length());
  }

  // Constructor from a Napi::String to an externally allocated buffer
  CobhanBufferNapi(const Napi::Env &env, const Napi::String &napiString,
                   char *cbuffer, size_t allocation_size)
      : CobhanBuffer(cbuffer, allocation_size), env(env) {
    copy_from_string(env, napiString);
  }

  // Constructor from a Napi::String to an externally allocated buffer
  CobhanBufferNapi(const Napi::Env &env, const Napi::Value &napiValue,
                   char *cbuffer, size_t allocation_size)
      : CobhanBuffer(cbuffer, allocation_size), env(env) {
    if (napiValue.IsString()) {
      copy_from_string(env, napiValue.As<Napi::String>());
    } else if (napiValue.IsBuffer()) {
      auto napiBuffer =
          napiValue.As<Napi::Buffer<unsigned char>>();
      std::memcpy(get_data_ptr(), napiBuffer.Data(), napiBuffer.Length());
    } else {
      NapiUtils::ThrowException(
          env, "Expected a Napi::String or Napi::Buffer<unsigned "
               "char> as the value.");
    }
  }

  // Constructor from size_t representing data length in bytes (not allocation
  // size)
  explicit CobhanBufferNapi(const Napi::Env &env, size_t data_len_bytes)
      : CobhanBuffer(data_len_bytes), env(env) {}

  // Constructor from externally allocated char* and size_t representing
  // allocation size in bytes
  explicit CobhanBufferNapi(const Napi::Env &env, char *cbuffer, size_t allocation_size)
      : CobhanBuffer(cbuffer, allocation_size), env(env) {}

  // Move constructor
  CobhanBufferNapi(CobhanBufferNapi &&other) noexcept
      : CobhanBuffer(std::move(other)), env(other.env) {}

  // Move assignment operator
  CobhanBufferNapi &operator=(CobhanBufferNapi &&other) noexcept {
    if (this != &other) {
      CobhanBuffer::operator=(std::move(other));
    }
    return *this;
  }

  // Returns a Napi::String from the buffer using napi_create_string_utf8
  [[nodiscard]] Napi::String ToString(const Napi::Env &) const {
    napi_value napiStr;
    napi_status status = napi_create_string_utf8(
        env, get_data_ptr(), get_data_len_bytes(), &napiStr);

    if (status != napi_ok) {
      napi_throw_error(env, nullptr,
                       "Failed to create Napi::String from CobhanBuffer");
      return {};
    }

    return {env, napiStr};
  }

  // Returns a Napi::Buffer<unsigned char> from the buffer
  [[nodiscard]] Napi::Buffer<unsigned char> ToBuffer(const Napi::Env &) const {
    auto buffer = Napi::Buffer<unsigned char>::Copy(
        env, reinterpret_cast<unsigned char *>(get_data_ptr()),
        get_data_len_bytes());
    return buffer;
  }

  // Public method to calculate the required allocation size for a Napi::String
  static size_t StringToAllocationSize(const Napi::Env &env,
                                       const Napi::String &napiString) {
    size_t str_len = NapiUtils::GetUtf8StringLength(env, napiString);
    return DataSizeToAllocationSize(str_len) +
           1; // Add one for possible NULL delimiter due to Node string
              // functions
  }

  // Public method to calculate the required allocation size for a
  // Napi::Buffer<unsigned char>
  static size_t
  BufferToAllocationSize(const Napi::Env &,
                         const Napi::Buffer<unsigned char> &napiBuffer) {
    return DataSizeToAllocationSize(napiBuffer.Length());
  }

  // Public method to calculate the required allocation size for a Napi::Value
  // (either Napi::String or Napi::Buffer<unsigned char>)
  static size_t ValueToAllocationSize(const Napi::Env &env,
                                      const Napi::Value &value) {
    if (value.IsString()) {
      return StringToAllocationSize(env, value.As<Napi::String>());
    } else if (value.IsBuffer()) {
      return BufferToAllocationSize(env,
                                    value.As<Napi::Buffer<unsigned char>>());
    } else {
      NapiUtils::ThrowException(
          env, "Expected a Napi::String or Napi::Buffer<unsigned "
               "char> as the value.");
    }
  }

  static size_t ValueToDataSize(const Napi::Env &env,
                                const Napi::Value &value) {
    if (value.IsString()) {
      return NapiUtils::GetUtf8StringLength(env, value.As<Napi::String>()) + 1;
    } else if (value.IsBuffer()) {
      return value.As<Napi::Buffer<unsigned char>>().ByteLength();
    } else {
      NapiUtils::ThrowException(
          env, "Expected a Napi::String or Napi::Buffer<unsigned "
               "char> as the value.");
    }
  }

private:
  Napi::Env env;

  void copy_from_string(const Napi::Env &, const Napi::String &napiString) {
    size_t str_len = NapiUtils::GetUtf8StringLength(env, napiString);

    size_t allocation_size =
        CobhanBuffer::DataSizeToAllocationSize(str_len) + 1;

    if (allocation_size > get_allocation_size()) {
      NapiUtils::ThrowException(
          env,
          "Buffer allocation size is insufficient to hold the Napi::String.");
    }

    size_t bytes_written;
    napi_status status = napi_get_value_string_utf8(
        env, napiString, get_data_ptr(), str_len + 1, &bytes_written);
    if (status != napi_ok || bytes_written != str_len) {
      NapiUtils::ThrowException(
          env, "Failed to copy Napi::String into CobhanBuffer. Status: " +
                   std::to_string(status) +
                   ", Bytes Written: " + std::to_string(bytes_written));
    }

    set_data_len_bytes(str_len);
  }
};

#endif // COBHAN_BUFFER_NAPI_H
