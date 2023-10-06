#include "../lib/libasherah.h"
#include "cobhan_napi_interop.h"
#include "hints.h"
#include "logging.h"
#include <iostream>
#include <napi.h>

size_t max_stack_alloc_size = 2048;
int32_t setup_state = 0;

void setup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("setup", "called");
  }

  if (unlikely(setup_state == 1)) {
    log_error_and_throw(env, "setup", "setup called twice");
  }

  if (unlikely(info.Length() < 1)) {
    log_error_and_throw(env, "setup", "Wrong number of arguments");
  }

  Napi::String config;
  Napi::Object config_json;
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
    log_error_and_throw(env, "setup", "Wrong argument type");
  }

  Napi::String product_id = config_json.Get("ProductID").As<Napi::String>();
  Napi::String service_name = config_json.Get("ServiceName").As<Napi::String>();

  est_intermediate_key_overhead =
      product_id.Utf8Value().length() + service_name.Utf8Value().length();

  Napi::Value verbose = config_json.Get("Verbose");
  if (likely(verbose.IsBoolean())) {
    verbose_flag = verbose.As<Napi::Boolean>().Value();
    debug_log("setup", "verbose_flag: " + std::to_string(verbose_flag));
  } else {
    verbose_flag = 0;
    debug_log("setup", "verbose_flag: defaulting to false");
  }

  char *config_cobhan_buffer;
  size_t config_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(config, config_cobhan_buffer, config_copied_bytes,
                         "setup");

  char *config_canary_ptr = get_canary_ptr(config_cobhan_buffer);
  if (unlikely(!check_canary_ptr(config_canary_ptr))) {
    log_error_and_throw(env, "encrypt_to_json",
                        "Failed initial canary check for config_cobhan_buffer");
  }

  // extern GoInt32 SetupJson(void* configJson);
  GoInt32 result = SetupJson(config_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("setup", "Returned from asherah-cobhan SetupJson");
  }

  if (unlikely(!check_canary_ptr(config_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for config_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    log_error_and_throw(env, "setup", std::to_string(result));
  }
  setup_state = 1;
}

Napi::String encrypt_to_json(Napi::Env &env, size_t partition_bytes,
                             size_t data_bytes,
                             char *partition_id_cobhan_buffer,
                             char *input_cobhan_buffer) {

  size_t asherah_output_size_bytes =
      estimate_asherah_output_size_bytes(data_bytes, partition_bytes);

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_to_json", " asherah_output_size_bytes " +
                                     std::to_string(asherah_output_size_bytes));
  }

  char *output_cobhan_buffer;
  ALLOCATE_OUTPUT_CBUFFER(output_cobhan_buffer, asherah_output_size_bytes,
                          "encrypt_to_json");

  char *partition_id_canary_ptr = get_canary_ptr(partition_id_cobhan_buffer);
  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed initial canary check for partition_id_cobhan_buffer");
  }
  char *input_canary_ptr = get_canary_ptr(input_cobhan_buffer);
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(env, "encrypt_to_json",
                        "Failed initial canary check for input_cobhan_buffer");
  }
  char *output_canary_ptr = get_canary_ptr(output_cobhan_buffer);
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(env, "encrypt_to_json",
                        "Failed initial canary check for output_cobhan_buffer");
  }

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_to_json", "Calling asherah-cobhan EncryptToJson");
  }

  // extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void*
  // jsonPtr);
  GoInt32 result = EncryptToJson(partition_id_cobhan_buffer,
                                 input_cobhan_buffer, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_to_json", "Returning from asherah-cobhan EncryptToJson");
  }

  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for partition_id_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for input_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for output_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    log_error_and_throw(env, "encrypt_to_json", std::to_string(result));
  }

  Napi::String output = cbuffer_to_nstring(env, output_cobhan_buffer);
  return output;
}

Napi::String encrypt(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("encrypt", "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(env, "encrypt", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(env, "encrypt", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsBuffer())) {
    log_error_and_throw(env, "encrypt", "Wrong argument types");
  }

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, "encrypt");

  Napi::Buffer<unsigned char> input_napi_buffer =
      info[1].As<Napi::Buffer<unsigned char>>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes = 0;
  NAPI_BUFFER_TO_CBUFFER(input_napi_buffer, input_cobhan_buffer,
                         input_copied_bytes, "encrypt");

  Napi::String output =
      encrypt_to_json(env, partition_id_copied_bytes, input_copied_bytes,
                      partition_id_cobhan_buffer, input_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("encrypt", "finished");
  }

  return output;
}

Napi::String encrypt_string(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_string", "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(env, "encrypt_string", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(env, "encrypt_string", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    log_error_and_throw(env, "encrypt_string", "Wrong argument types");
  }

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, "encrypt_string");

  Napi::String input = info[1].As<Napi::String>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(input, input_cobhan_buffer, input_copied_bytes,
                         "encrypt_string");

  Napi::String output =
      encrypt_to_json(env, partition_id_copied_bytes, input_copied_bytes,
                      partition_id_cobhan_buffer, input_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_string", "finished");
  }

  return output;
}

Napi::Buffer<unsigned char> decrypt(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("decrypt", "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(env, "decrypt", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(env, "decrypt", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    log_error_and_throw(env, "decrypt", "Wrong argument types");
  }

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, "decrypt");

  Napi::String input = info[1].As<Napi::String>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(input, input_cobhan_buffer, input_copied_bytes,
                         "decrypt");

  char *output_cobhan_buffer;
  ALLOCATE_OUTPUT_CBUFFER(output_cobhan_buffer, input_copied_bytes, "decrypt");

  char *partition_id_canary_ptr = get_canary_ptr(partition_id_cobhan_buffer);
  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed initial canary check for partition_id_cobhan_buffer");
  }
  char *input_canary_ptr = get_canary_ptr(input_cobhan_buffer);
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(env, "encrypt_to_json",
                        "Failed initial canary check for input_cobhan_buffer");
  }
  char *output_canary_ptr = get_canary_ptr(output_cobhan_buffer);
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(env, "encrypt_to_json",
                        "Failed initial canary check for output_cobhan_buffer");
  }

  if (unlikely(verbose_flag)) {
    debug_log("decrypt", "Calling asherah-cobhan DecryptFromJson");
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partition_id_cobhan_buffer,
                                   input_cobhan_buffer, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("decrypt", "Returned from asherah-cobhan DecryptFromJson");
  }

  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for partition_id_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for input_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for output_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    log_error_and_throw(env, "decrypt", std::to_string(result));
  }

  Napi::Buffer<unsigned char> output =
      cbuffer_to_nbuffer(env, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("decrypt", "finished");
  }

  return output;
}

Napi::String decrypt_string(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("decrypt_string", "called");
  }

  if (unlikely(setup_state == 0)) {
    log_error_and_throw(env, "decrypt_string", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    log_error_and_throw(env, "decrypt_string", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    log_error_and_throw(env, "decrypt_string", "Wrong argument types");
  }

  Napi::String partition_id = info[0].As<Napi::String>();
  char *partition_id_cobhan_buffer;
  size_t partition_id_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(partition_id, partition_id_cobhan_buffer,
                         partition_id_copied_bytes, "decrypt_string");

  Napi::String input = info[1].As<Napi::String>();
  char *input_cobhan_buffer;
  size_t input_copied_bytes = 0;
  NAPI_STRING_TO_CBUFFER(input, input_cobhan_buffer, input_copied_bytes,
                         "decrypt_string");

  char *output_cobhan_buffer;
  ALLOCATE_OUTPUT_CBUFFER(output_cobhan_buffer, input_copied_bytes,
                          "decrypt_string");

  char *partition_id_canary_ptr = get_canary_ptr(partition_id_cobhan_buffer);
  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed initial canary check for partition_id_cobhan_buffer");
  }
  char *input_canary_ptr = get_canary_ptr(input_cobhan_buffer);
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(env, "encrypt_to_json",
                        "Failed initial canary check for input_cobhan_buffer");
  }
  char *output_canary_ptr = get_canary_ptr(output_cobhan_buffer);
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(env, "encrypt_to_json",
                        "Failed initial canary check for output_cobhan_buffer");
  }

  if (unlikely(verbose_flag)) {
    debug_log("decrypt_string", "Calling asherah-cobhan DecryptFromJson");
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partition_id_cobhan_buffer,
                                   input_cobhan_buffer, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("decrypt_string", "Returned from asherah-cobhan DecryptFromJson");
  }

  if (unlikely(!check_canary_ptr(partition_id_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for partition_id_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(input_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for input_cobhan_buffer");
  }
  if (unlikely(!check_canary_ptr(output_canary_ptr))) {
    log_error_and_throw(
        env, "encrypt_to_json",
        "Failed post-call canary check for output_cobhan_buffer");
  }

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    log_error_and_throw(env, "decrypt_string", std::to_string(result));
  }

  Napi::String output = cbuffer_to_nstring(env, output_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("decrypt_string", "finished");
  }

  return output;
}

void shutdown(const Napi::CallbackInfo &info) {
  if (unlikely(verbose_flag)) {
    debug_log("shutdown", "called");
  }

  setup_state = 0;

  if (unlikely(verbose_flag)) {
    debug_log("shutdown", "Calling asherah-cobhan Shutdown");
  }

  // extern void Shutdown();
  Shutdown();

  if (unlikely(verbose_flag)) {
    debug_log("shutdown", "Returned from asherah-cobhan Shutdown");
  }
}

void set_max_stack_alloc_item_size(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("set_max_stack_alloc_item_size", "called");
  }

  if (unlikely(info.Length() < 1)) {
    log_error_and_throw(env, "set_max_stack_alloc_item_size",
                        "Wrong number of arguments");
  }

  Napi::Number item_size = info[0].ToNumber();

  max_stack_alloc_size = (size_t)item_size.Int32Value();
}

void set_safety_padding_overhead(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("set_safety_padding_overhead", "called");
  }

  if (unlikely(info.Length() < 1)) {
    log_error_and_throw(env, "set_safety_padding_overhead",
                        "Wrong number of arguments");
  }

  Napi::Number safety_padding_number = info[0].ToNumber();

  safety_padding_bytes = (size_t)safety_padding_number.Int32Value();
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
