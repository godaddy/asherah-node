#include "asherah.h"
#include "../lib/libasherah.h"
#include "cobhan_napi_interop.h"
#include "hints.h"
#include "logging.h"
#include <iostream>
#include <mutex>
#include <napi.h>

size_t est_intermediate_key_overhead;
size_t maximum_stack_alloc_size = 2048;

int32_t setup_state = 0;
std::mutex asherah_lock;

void setup(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  if (unlikely(setup_state == 1)) {
    log_error_and_throw(__func__, "setup called twice");
  }

  if (unlikely(info.Length() < 1)) {
    log_error_and_throw(__func__, "Wrong number of arguments");
  }

  Napi::String config;
  Napi::Object config_json;
  Napi::Env env = info.Env();
  Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
  if (likely(info[0].IsObject())) {
    config_json = info[0].As<Napi::Object>();
    Napi::Function stringify = json.Get("stringify").As<Napi::Function>();
    config = stringify.Call(json, {config_json}).As<Napi::String>();
  } else if (likely(info[0].IsString())) {
    config = info[0].As<Napi::String>();
    Napi::Function parse = json.Get("parse").As<Napi::Function>();
    config_json = parse.Call(json, {config}).As<Napi::Object>();
  } else {
    log_error_and_throw(__func__, "Wrong argument type");
  }

  Napi::String product_id = config_json.Get("ProductID").As<Napi::String>();
  Napi::String service_name = config_json.Get("ServiceName").As<Napi::String>();

  est_intermediate_key_overhead =
      product_id.Utf8Value().length() + service_name.Utf8Value().length();

  Napi::Value verbose = config_json.Get("Verbose");
  if (likely(verbose.IsBoolean())) {
    verbose_flag = verbose.As<Napi::Boolean>().Value();
    debug_log(__func__, "verbose_flag: " + std::to_string(verbose_flag));
  } else {
    verbose_flag = 0;
    debug_log(__func__, "verbose_flag: defaulting to false");
  }

  char *config_cobhan_buffer;
  size_t config_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, config, config_cobhan_buffer, config_copied_bytes,
                         maximum_stack_alloc_size, __func__);

  char *config_canary_ptr = get_canary_ptr(config_cobhan_buffer);
  if (unlikely(!check_canary_ptr(config_canary_ptr))) {
    log_error_and_throw(__func__,
                        "Failed initial canary check for config_cobhan_buffer");
  }

  // extern GoInt32 SetupJson(void* configJson);
  GoInt32 result = SetupJson(config_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Returned from asherah-cobhan SetupJson");
  }

  if (unlikely(!check_canary_ptr(config_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed post-call canary check for config_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    log_error_and_throw(__func__, asherah_cobhan_error_to_string(result));
  }
  setup_state = 1;
}

Napi::String encrypt(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(__func__, "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(__func__, "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsBuffer())) {
    log_error_and_throw(__func__, "Wrong argument types");
  }

  Napi::Env env = info.Env();

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, maximum_stack_alloc_size,
                         __func__);

  Napi::Buffer<unsigned char> input_napi_buffer =
      info[1].As<Napi::Buffer<unsigned char>>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes;
  NAPI_BUFFER_TO_CBUFFER(env, input_napi_buffer, input_cobhan_buffer,
                         input_copied_bytes, maximum_stack_alloc_size,
                         __func__);

  Napi::String output =
      encrypt_to_json(env, partition_id_copied_bytes, input_copied_bytes,
                      partition_id_cobhan_buffer, input_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "finished");
  }

  return output;
}

Napi::String encrypt_string(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(__func__, "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(__func__, "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    log_error_and_throw(__func__, "Wrong argument types");
  }

  Napi::Env env = info.Env();

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, maximum_stack_alloc_size,
                         __func__);

  Napi::String input = info[1].As<Napi::String>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, input, input_cobhan_buffer, input_copied_bytes,
                         maximum_stack_alloc_size, __func__);

  Napi::String output =
      encrypt_to_json(env, partition_id_copied_bytes, input_copied_bytes,
                      partition_id_cobhan_buffer, input_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "finished");
  }

  return output;
}

Napi::String encrypt_to_json(Napi::Env &env, size_t partition_bytes,
                             size_t data_bytes,
                             char *partition_id_cobhan_buffer,
                             char *input_cobhan_buffer) {

  size_t asherah_output_size_bytes =
      estimate_asherah_output_size_bytes(data_bytes, partition_bytes);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, " asherah_output_size_bytes " +
                            std::to_string(asherah_output_size_bytes));
  }

  char *output_cobhan_buffer;
  ALLOCATE_CBUFFER(output_cobhan_buffer, asherah_output_size_bytes,
                   maximum_stack_alloc_size, __func__);

  char *partition_id_canary_ptr = get_canary_ptr(partition_id_cobhan_buffer);
  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed initial canary check for partition_id_cobhan_buffer");
  }
  char *input_canary_ptr = get_canary_ptr(input_cobhan_buffer);
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(__func__,
                        "Failed initial canary check for input_cobhan_buffer");
  }
  char *output_canary_ptr = get_canary_ptr(output_cobhan_buffer);
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(__func__,
                        "Failed initial canary check for output_cobhan_buffer");
  }

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Calling asherah-cobhan EncryptToJson");
  }

  // extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void*
  // jsonPtr);
  GoInt32 result = EncryptToJson(partition_id_cobhan_buffer,
                                 input_cobhan_buffer, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Returning from asherah-cobhan EncryptToJson");
  }

  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        __func__,
        "Failed post-call canary check for partition_id_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed post-call canary check for input_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed post-call canary check for output_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    log_error_and_throw(__func__, asherah_cobhan_error_to_string(result));
  }

  Napi::String output = cbuffer_to_nstring(env, output_cobhan_buffer);
  return output;
}

Napi::Buffer<unsigned char> decrypt(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(__func__, "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(__func__, "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    log_error_and_throw(__func__, "Wrong argument types");
  }

  Napi::Env env = info.Env();

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, maximum_stack_alloc_size,
                         __func__);

  Napi::String input = info[1].As<Napi::String>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, input, input_cobhan_buffer, input_copied_bytes,
                         maximum_stack_alloc_size, __func__);

  char *output_cobhan_buffer;
  ALLOCATE_CBUFFER(output_cobhan_buffer, input_copied_bytes,
                   maximum_stack_alloc_size, __func__);

  char *partition_id_canary_ptr = get_canary_ptr(partition_id_cobhan_buffer);
  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed initial canary check for partition_id_cobhan_buffer");
  }
  char *input_canary_ptr = get_canary_ptr(input_cobhan_buffer);
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(__func__,
                        "Failed initial canary check for input_cobhan_buffer");
  }
  char *output_canary_ptr = get_canary_ptr(output_cobhan_buffer);
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(__func__,
                        "Failed initial canary check for output_cobhan_buffer");
  }

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Calling asherah-cobhan DecryptFromJson");
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partition_id_cobhan_buffer,
                                   input_cobhan_buffer, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Returned from asherah-cobhan DecryptFromJson");
  }

  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        __func__,
        "Failed post-call canary check for partition_id_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed post-call canary check for input_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed post-call canary check for output_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    log_error_and_throw(__func__, asherah_cobhan_error_to_string(result));
  }

  Napi::Buffer<unsigned char> output =
      cbuffer_to_nbuffer(env, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "finished");
  }

  return output;
}

Napi::String decrypt_string(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(__func__, "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(__func__, "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    log_error_and_throw(__func__, "Wrong argument types");
  }

  Napi::Env env = info.Env();

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, maximum_stack_alloc_size,
                         __func__);

  Napi::String input = info[1].As<Napi::String>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes;
  NAPI_STRING_TO_CBUFFER(env, input, input_cobhan_buffer, input_copied_bytes,
                         maximum_stack_alloc_size, __func__);

  char *output_cobhan_buffer;
  ALLOCATE_CBUFFER(output_cobhan_buffer, input_copied_bytes,
                   maximum_stack_alloc_size, __func__);

  char *partition_id_canary_ptr = get_canary_ptr(partition_id_cobhan_buffer);
  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed initial canary check for partition_id_cobhan_buffer");
  }
  char *input_canary_ptr = get_canary_ptr(input_cobhan_buffer);
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(__func__,
                        "Failed initial canary check for input_cobhan_buffer");
  }
  char *output_canary_ptr = get_canary_ptr(output_cobhan_buffer);
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(__func__,
                        "Failed initial canary check for output_cobhan_buffer");
  }

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Calling asherah-cobhan DecryptFromJson");
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partition_id_cobhan_buffer,
                                   input_cobhan_buffer, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Returned from asherah-cobhan DecryptFromJson");
  }

  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        __func__,
        "Failed post-call canary check for partition_id_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed post-call canary check for input_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(
        __func__, "Failed post-call canary check for output_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    log_error_and_throw(__func__, asherah_cobhan_error_to_string(result));
  }

  Napi::String output = cbuffer_to_nstring(env, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "finished");
  }

  return output;
}

void shutdown(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  setup_state = 0;

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Calling asherah-cobhan Shutdown");
  }

  // extern void Shutdown();
  Shutdown();

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Returned from asherah-cobhan Shutdown");
  }
}

void set_max_stack_alloc_item_size(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  if (unlikely(info.Length() < 1)) {
    log_error_and_throw(__func__, "Wrong number of arguments");
  }

  Napi::Number item_size = info[0].ToNumber();

  maximum_stack_alloc_size = (size_t)item_size.Int32Value();
}

void set_safety_padding_overhead(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(asherah_lock);

  if (unlikely(verbose_flag)) {
    debug_log(__func__, "called");
  }

  if (unlikely(info.Length() < 1)) {
    log_error_and_throw(__func__, "Wrong number of arguments");
  }

  Napi::Number safety_padding_number = info[0].ToNumber();

  set_safety_padding_bytes((size_t)safety_padding_number.Int32Value());
}

__attribute__((always_inline)) inline size_t
estimate_asherah_output_size_bytes(size_t data_byte_len,
                                   size_t partition_byte_len) {
  const size_t est_encryption_overhead = 48;
  const size_t est_envelope_overhead = 185;
  const double base64_overhead = 1.34;

  // Add one rather than using std::ceil to round up
  double est_data_byte_len =
      (double(data_byte_len + est_encryption_overhead) * base64_overhead) + 1;

  size_t asherah_output_size_bytes =
      size_t(est_envelope_overhead + est_intermediate_key_overhead +
             partition_byte_len + est_data_byte_len);
  if (unlikely(verbose_flag)) {
    std::string log_msg =
        "estimate_asherah_output_size(" + std::to_string(data_byte_len) + ", " +
        std::to_string(partition_byte_len) +
        ") est_data_byte_len: " + std::to_string(est_data_byte_len) +
        " asherah_output_size_bytes: " +
        std::to_string(asherah_output_size_bytes);
    debug_log(__func__, log_msg);
  }
  return asherah_output_size_bytes;
}

__attribute__((always_inline)) inline const char *
asherah_cobhan_error_to_string(int32_t error) {
  switch (error) {
  case 0:
    return "Success";
  case -1:
    return "Cobhan error: NULL pointer";
  case -2:
    return "Cobhan error: Buffer too large";
  case -3:
    return "Cobhan error: Buffer too small";
  case -4:
    return "Cobhan error: Copy failed";
  case -5:
    return "Cobhan error: JSON decode failed";
  case -6:
    return "Cobhan error: JSON encode failed";
  case -7:
    return "Cobhan error: Invalid UTF-8";
  case -8:
    return "Cobhan error: Read temp file failed";
  case -9:
    return "Cobhan error: Write temp file failed";
  case -100:
    return "Asherah error: Not initialized";
  case -101:
    return "Asherah error: Already initialized";
  case -102:
    return "Asherah error: Failed to get session";
  case -103:
    return "Asherah error: Encrypt operation failed";
  case -104:
    return "Asherah error: Decrypt operation failed";
  case -105:
    return "Asherah error: Invalid configuration";
  default:
    return "Unknown error";
  }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "setup"), Napi::Function::New(env, setup));
  exports.Set(Napi::String::New(env, "encrypt"),
              Napi::Function::New(env, encrypt));
  exports.Set(Napi::String::New(env, "encrypt_string"),
              Napi::Function::New(env, encrypt_string));
  exports.Set(Napi::String::New(env, "decrypt"),
              Napi::Function::New(env, decrypt));
  exports.Set(Napi::String::New(env, "decrypt_string"),
              Napi::Function::New(env, decrypt_string));
  exports.Set(Napi::String::New(env, "set_safety_padding_overhead"),
              Napi::Function::New(env, set_safety_padding_overhead));
  exports.Set(Napi::String::New(env, "set_max_stack_alloc_item_size"),
              Napi::Function::New(env, set_max_stack_alloc_item_size));
  exports.Set(Napi::String::New(env, "shutdown"),
              Napi::Function::New(env, shutdown));
  return exports;
}

NODE_API_MODULE(asherah, Init)
