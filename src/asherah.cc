#include "../lib/libasherah.h"
#include <iostream>
#define NODE_ADDON_API_DISABLE_DEPRECATED
#include "cobhan_napi_interop.h"
#include "hints.h"
#include "logging.h"
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
    return;
  }

  if (unlikely(info.Length() < 1)) {
    log_error_and_throw(env, "setup", "Wrong number of arguments");
    return;
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
    return;
  }

  Napi::String product_id = config_json.Get("ProductID").As<Napi::String>();
  Napi::String service_name = config_json.Get("ServiceName").As<Napi::String>();

  set_est_intermediate_key_overhead(product_id.Utf8Value().length() + service_name.Utf8Value().length());

  Napi::Value verbose = config_json.Get("Verbose");
  if (likely(verbose.IsBoolean())) {
    verbose_flag = verbose.As<Napi::Boolean>().Value();
    debug_log("setup", "verbose_flag: " + std::to_string(verbose_flag));
  } else {
    verbose_flag = 0;
    debug_log("setup", "verbose_flag: defaulting to false");
  }

  // Determine size
  size_t config_utf8_byte_length;
  config_utf8_byte_length = nstring_utf8_byte_length(env, config);
  if (unlikely(config_utf8_byte_length == (size_t)(-1))) {
    log_error_and_throw(env, "setup", "Failed to get config utf8 length");
    return;
  }

  // Allocate
  char *config_cobhan_buffer;
  std::unique_ptr<char[]> config_cobhan_buffer_unique_ptr;
  if (config_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t config_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(config_utf8_byte_length);
    debug_log_alloca("setup", "config_cobhan_buffer",
                     config_cobhan_buffer_size_bytes);
    config_cobhan_buffer = (char *)alloca(config_cobhan_buffer_size_bytes);
    configure_cbuffer(config_cobhan_buffer, config_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    config_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("config_cobhan_buffer", config_utf8_byte_length);
    config_cobhan_buffer = config_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(config_cobhan_buffer == nullptr)) {
    log_error_and_throw(env, "setup", "Failed to allocate config cobhan buffer");
    return;
  }

  // Copy
  size_t config_copied_bytes;
  config_cobhan_buffer =
      copy_nstring_to_cbuffer(env, config, config_utf8_byte_length,
                              config_cobhan_buffer, &config_copied_bytes);
  if (unlikely(config_cobhan_buffer == nullptr)) {
    log_error_and_throw(env, "setup", "Failed to copy config to cobhan buffer");
    return;
  }

  if (unlikely(verbose_flag)) {
    debug_log("setup", "Calling asherah-cobhan SetupJson");
  }

  // extern GoInt32 SetupJson(void* configJson);
  GoInt32 result = SetupJson(config_cobhan_buffer);

  if (unlikely(verbose_flag)) {
    debug_log("setup", "Returned from asherah-cobhan SetupJson");
  }

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    log_error_and_throw(env, "setup", std::to_string(result));
    return;
  }
  setup_state = 1;
}

Napi::Value encrypt_to_json(Napi::Env &env, size_t partition_bytes,
                            size_t data_bytes, char *partition_id_cobhan_buffer,
                            char *input_cobhan_buffer) {

  size_t asherah_output_size_bytes =
      estimate_asherah_output_size_bytes(data_bytes, partition_bytes);

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_to_json", " asherah_output_size_bytes " +
                                     std::to_string(asherah_output_size_bytes));
  }

  char *output_cobhan_buffer;
  std::unique_ptr<char[]> output_cobhan_buffer_unique_ptr;
  if (asherah_output_size_bytes < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t output_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(asherah_output_size_bytes);
    debug_log_alloca("encrypt_to_json", "output_cobhan_buffer",
                     output_cobhan_buffer_size_bytes);
    output_cobhan_buffer = (char *)alloca(output_cobhan_buffer_size_bytes);
    configure_cbuffer(output_cobhan_buffer, asherah_output_size_bytes);
  } else {
    // Otherwise, allocate it on the heap
    output_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "output_cobhan_buffer", asherah_output_size_bytes);
    output_cobhan_buffer = output_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(output_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt_to_json",
                            "Failed to allocate cobhan output buffer");
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

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    return log_error_and_throw(env, "encrypt_to_json", std::to_string(result));
  }

  Napi::Value output = cbuffer_to_nstring(env, output_cobhan_buffer);
  return output;
}

Napi::Value encrypt(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("encrypt", "called");
  }

  if (unlikely(setup_state == 0)) {
    return log_error_and_throw(env, "encrypt", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return log_error_and_throw(env, "encrypt", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsBuffer())) {
    return log_error_and_throw(env, "encrypt", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return log_error_and_throw(env, "encrypt",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return log_error_and_throw(env, "encrypt", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("encrypt", "partition_id_cobhan_buffer",
                     partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer =
        (char *)alloca(partition_id_cobhan_buffer_size_bytes);
    configure_cbuffer(partition_id_cobhan_buffer, partition_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt",
                            "Failed to allocate partitionId cobhan buffer");
  }

  // Copy
  size_t partition_copied_bytes;
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt",
                            "Failed to copy partitionId to cobhan buffer");
  }

  // Determine size
  Napi::Buffer<unsigned char> input_napi_buffer =
      info[1].As<Napi::Buffer<unsigned char>>();
  size_t input_byte_length = input_napi_buffer.ByteLength();
  if (unlikely(input_byte_length == 0)) {
    return log_error_and_throw(env, "encrypt", "input is empty");
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_buffer_unique_ptr;
  if (input_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_byte_length);
    debug_log_alloca("encrypt", "input_cobhan_buffer",
                     input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
    configure_cbuffer(input_cobhan_buffer, input_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    input_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_byte_length);
    input_cobhan_buffer = input_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(
        env, "encrypt", "Failed to allocate cobhan buffer for input buffer");
  }

  // Copy
  input_cobhan_buffer =
      copy_nbuffer_to_cbuffer(env, input_napi_buffer, input_cobhan_buffer);
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt",
                            "Failed to copy input buffer to cobhan buffer");
  }

  return encrypt_to_json(env, partition_copied_bytes, input_byte_length,
                         partition_id_cobhan_buffer, input_cobhan_buffer);
}

Napi::Value encrypt_string(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_string", "called");
  }

  if (unlikely(setup_state == 0)) {
    return log_error_and_throw(env, "encrypt_string", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return log_error_and_throw(env, "encrypt_string", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return log_error_and_throw(env, "encrypt_string", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return log_error_and_throw(env, "encrypt_string",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return log_error_and_throw(env, "encrypt_string", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("encrypt_string", "partition_id_cobhan_buffer",
                     partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer =
        (char *)alloca(partition_id_cobhan_buffer_size_bytes);
    configure_cbuffer(partition_id_cobhan_buffer, partition_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt_string",
                            "Failed to allocate partitionId cobhan buffer");
  }

  // Copy
  size_t partition_copied_bytes;
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt_string",
                            "Failed to copy partitionId to cobhan buffer");
  }

  // Determine size
  size_t input_utf8_byte_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_byte_length = nstring_utf8_byte_length(env, input);
  if (unlikely(input_utf8_byte_length == (size_t)(-1))) {
    return log_error_and_throw(env, "encrypt_string",
                            "Failed to get input utf8 length");
  }
  if (unlikely(input_utf8_byte_length == 0)) {
    return log_error_and_throw(env, "encrypt_string", "input is empty");
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("encrypt_string", "input_cobhan_buffer",
                     input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
    configure_cbuffer(input_cobhan_buffer, input_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    input_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_utf8_byte_length);
    input_cobhan_buffer = input_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt_string",
                            "Failed to allocate input cobhan buffer");
  }

  // Copy
  size_t input_copied_bytes;
  input_cobhan_buffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_byte_length,
                              input_cobhan_buffer, &input_copied_bytes);
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "encrypt_string",
                            "Failed to copy input to cobhan buffer");
  }

  return encrypt_to_json(env, partition_copied_bytes, input_utf8_byte_length,
                         partition_id_cobhan_buffer, input_cobhan_buffer);
}

Napi::Value decrypt(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("decrypt", "called");
  }

  if (unlikely(setup_state == 0)) {
    return log_error_and_throw(env, "decrypt", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return log_error_and_throw(env, "decrypt", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return log_error_and_throw(env, "decrypt", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length, partition_copied_bytes;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return log_error_and_throw(env, "decrypt",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return log_error_and_throw(env, "decrypt", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("decrypt", "partition_id_cobhan_buffer",
                     partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer =
        (char *)alloca(partition_id_cobhan_buffer_size_bytes);
    configure_cbuffer(partition_id_cobhan_buffer, partition_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt",
                            "Failed to allocate partition_id cobhan buffer");
  }

  // Copy
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt",
                            "Failed to copy partition_id to cobhan buffer");
  }

  // Determine size
  size_t input_utf8_byte_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_byte_length = nstring_utf8_byte_length(env, input);
  if (unlikely(input_utf8_byte_length == (size_t)(-1))) {
    return log_error_and_throw(env, "decrypt", "Failed to get input utf8 length");
  }
  if (unlikely(input_utf8_byte_length == 0)) {
    return log_error_and_throw(env, "decrypt", "input is empty");
  }

  if (unlikely(verbose_flag)) {
    debug_log("decrypt",
              "input size " + std::to_string(input_utf8_byte_length));
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt", "input_cobhan_buffer",
                     input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
    configure_cbuffer(input_cobhan_buffer, input_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    input_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_utf8_byte_length);
    input_cobhan_buffer = input_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt",
                            "Failed to allocate input cobhan buffer");
  }

  // Copy
  size_t input_copied_bytes;
  input_cobhan_buffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_byte_length,
                              input_cobhan_buffer, &input_copied_bytes);
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt",
                            "Failed to copy input to cobhan buffer");
  }

  char *output_cobhan_buffer;
  std::unique_ptr<char[]> output_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t output_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt", "output_cobhan_buffer",
                     output_cobhan_buffer_size_bytes);
    output_cobhan_buffer = (char *)alloca(output_cobhan_buffer_size_bytes);
    configure_cbuffer(output_cobhan_buffer, input_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    output_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("output_cobhan_buffer", input_utf8_byte_length);
    output_cobhan_buffer = output_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(output_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt",
                            "Failed to allocate cobhan output buffer");
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

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    return log_error_and_throw(env, "decrypt", std::to_string(result));
  }

  return cbuffer_to_nbuffer(env, output_cobhan_buffer);
}

Napi::Value decrypt_string(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("decrypt_string", "called");
  }

  if (unlikely(setup_state == 0)) {
    return log_error_and_throw(env, "decrypt_string", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return log_error_and_throw(env, "decrypt_string", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return log_error_and_throw(env, "decrypt_string", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return log_error_and_throw(env, "decrypt_string",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return log_error_and_throw(env, "decrypt_string", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("decrypt_string", "partition_id_cobhan_buffer",
                     partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer =
        (char *)alloca(partition_id_cobhan_buffer_size_bytes);
    configure_cbuffer(partition_id_cobhan_buffer, partition_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt_string",
                            "Failed to allocate partitionId cobhan buffer");
  }

  // Copy
  size_t partition_copied_bytes;
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt_string",
                            "Failed to copy partitionId to cobhan buffer");
  }

  // Determine size
  size_t input_utf8_byte_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_byte_length = nstring_utf8_byte_length(env, input);
  if (unlikely(input_utf8_byte_length == (size_t)(-1))) {
    return log_error_and_throw(env, "decrypt_string",
                            "Failed to get input utf8 length");
  }
  if (unlikely(input_utf8_byte_length == 0)) {
    return log_error_and_throw(env, "decrypt_string", "input is empty");
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt_string", "input_cobhan_buffer",
                     input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
    configure_cbuffer(input_cobhan_buffer, input_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    input_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_utf8_byte_length);
    input_cobhan_buffer = input_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt_string",
                            "Failed to allocate input cobhan buffer");
  }

  // Copy
  size_t input_copied_bytes;
  input_cobhan_buffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_byte_length,
                              input_cobhan_buffer, &input_copied_bytes);
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt_string",
                            "Failed to copy input to cobhan buffer");
  }

  char *output_cobhan_buffer;
  std::unique_ptr<char[]> output_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    // If the buffer is small enough, allocate it on the stack
    size_t output_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt_string", "output_cobhan_buffer",
                     output_cobhan_buffer_size_bytes);
    output_cobhan_buffer = (char *)alloca(output_cobhan_buffer_size_bytes);
    configure_cbuffer(output_cobhan_buffer, input_utf8_byte_length);
  } else {
    // Otherwise, allocate it on the heap
    output_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("output_cobhan_buffer", input_utf8_byte_length);
    output_cobhan_buffer = output_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(output_cobhan_buffer == nullptr)) {
    return log_error_and_throw(env, "decrypt_string",
                            "Failed to allocate cobhan output buffer");
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

  if (unlikely(result < 0)) {
    // TODO: Convert this to a proper error message
    return log_error_and_throw(env, "decrypt_string", std::to_string(result));
  }

  Napi::Value output = cbuffer_to_nstring(env, output_cobhan_buffer);
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
    return;
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
    return;
  }

  Napi::Number safety_padding_number = info[0].ToNumber();

  set_safety_padding_bytes((size_t)safety_padding_number.Int32Value());
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
