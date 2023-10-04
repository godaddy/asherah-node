#include "../lib/libasherah.h"
#include <iostream>
#define NODE_ADDON_API_DISABLE_DEPRECATED
#include <napi.h>

#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

const size_t cobhan_header_size_bytes = 64 / 8;
const size_t est_encryption_overhead = 48;
const size_t est_envelope_overhead = 185;
const double base64_overhead = 1.34;

size_t est_intermediate_key_overhead = 0;
size_t safety_padding_bytes = 0;
size_t max_stack_alloc_size = 2048;

int32_t setup_state = 0;
int32_t verbose_flag = 0;

__attribute__((always_inline)) inline void debug_log(const char *function_name,
                                                     std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void
debug_log_alloca(const char *function_name, const char *variable_name,
                 size_t length) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name
              << ": Calling alloca(" << length << ") (stack) for "
              << variable_name << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void error_log(const char *function_name,
                                                     std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [ERROR] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

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

__attribute__((always_inline)) inline Napi::Value
LogErrorAndThrow(Napi::Env &env, const char *function_name,
                 std::string error_msg) {
  error_log(function_name, error_msg);
  Napi::Error::New(env, function_name + (": " + error_msg))
      .ThrowAsJavaScriptException();
  return env.Null();
}

__attribute__((always_inline)) inline size_t
calculate_cobhan_buffer_size_bytes(size_t data_len_bytes) {
  return data_len_bytes + cobhan_header_size_bytes + safety_padding_bytes +
         1; // Add one for possible NULL delimiter due to Node string functions
}

__attribute__((always_inline)) inline size_t
estimate_asherah_output_size_bytes(size_t data_byte_len,
                                   size_t partition_byte_len) {
  // Add one rather than using std::ceil to round up
  double est_data_byte_len =
      (double(data_byte_len + est_encryption_overhead) * base64_overhead) + 1;

  size_t asherah_output_size_bytes =
      size_t(est_envelope_overhead + est_intermediate_key_overhead +
             partition_byte_len + est_data_byte_len + safety_padding_bytes);
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
  configure_cbuffer(cobhan_buffer, size_bytes + safety_padding_bytes);
  return cobhan_buffer_unique_ptr;
}

__attribute__((always_inline)) inline Napi::Value
cbuffer_to_nstring(Napi::Env &env, char *cobhan_buffer) {
  napi_value output;

  // Using C function because it allows length delimited input
  napi_status status = napi_create_string_utf8(
      env, ((const char *)cobhan_buffer) + cobhan_header_size_bytes,
      *((int *)cobhan_buffer), &output);

  if (unlikely(status != napi_ok)) {
    return LogErrorAndThrow(env, "cbuffer_to_nstring",
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
    LogErrorAndThrow(env, "nstring_utf8_length",
                     "napi_get_value_string_utf8 length check failed: " +
                         napi_status_to_string(status));
    return (size_t)(-1);
  }

  return utf8_length;
}

__attribute__((always_inline)) inline char *
copy_nstring_to_cbuffer(Napi::Env &env, Napi::String &str,
                        size_t utf8_byte_length, char *cobhan_buffer,
                        size_t *byte_length = nullptr) {
  napi_status status;
  size_t copied_bytes;
  status = napi_get_value_string_utf8(env, str,
                                      cobhan_buffer + cobhan_header_size_bytes,
                                      utf8_byte_length + 1, &copied_bytes);
  if (unlikely(status != napi_ok)) {
    LogErrorAndThrow(env, "copy_nstring_to_cbuffer",
                     "Napi utf8 string conversion failure: " +
                         napi_status_to_string(status));
    return nullptr;
  }

  if (unlikely(copied_bytes != utf8_byte_length)) {
    LogErrorAndThrow(env, "copy_nstring_to_cbuffer",
                     "Did not copy expected number of bytes " +
                         std::to_string(utf8_byte_length) + " copied " +
                         std::to_string(copied_bytes));
    return nullptr;
  }

  configure_cbuffer(cobhan_buffer, copied_bytes);

  if (byte_length != nullptr)
    *byte_length = copied_bytes;
  return cobhan_buffer;
}

__attribute__((always_inline)) inline Napi::Buffer<unsigned char>
cbuffer_to_nbuffer(Napi::Env &env, char *cobhan_buffer) {
  return Napi::Buffer<unsigned char>::Copy(
      env, ((unsigned char *)cobhan_buffer) + cobhan_header_size_bytes,
      *((int *)cobhan_buffer));
}

void setup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("setup", "called");
  }

  if (unlikely(setup_state == 1)) {
    LogErrorAndThrow(env, "setup", "setup called twice");
    return;
  }

  if (unlikely(info.Length() < 1)) {
    LogErrorAndThrow(env, "setup", "Wrong number of arguments");
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
    LogErrorAndThrow(env, "setup", "Wrong argument type");
    return;
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

  // Determine size
  size_t config_utf8_byte_length;
  config_utf8_byte_length = nstring_utf8_byte_length(env, config);
  if (unlikely(config_utf8_byte_length == (size_t)(-1))) {
    LogErrorAndThrow(env, "setup", "Failed to get config utf8 length");
    return;
  }

  // Allocate
  char *config_cobhan_buffer;
  std::unique_ptr<char[]> config_cobhan_buffer_unique_ptr;
  if (config_utf8_byte_length < max_stack_alloc_size) {
    size_t config_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(config_utf8_byte_length);
    debug_log_alloca("setup", "config_cobhan_buffer", config_cobhan_buffer_size_bytes);
    config_cobhan_buffer = (char *)alloca(config_cobhan_buffer_size_bytes);
  } else {
    config_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("config_cobhan_buffer", config_utf8_byte_length);
    config_cobhan_buffer = config_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(config_cobhan_buffer == nullptr)) {
    LogErrorAndThrow(env, "setup", "Failed to allocate config cobhan buffer");
    return;
  }

  // Copy
  size_t config_copied_bytes;
  config_cobhan_buffer =
      copy_nstring_to_cbuffer(env, config, config_utf8_byte_length,
                              config_cobhan_buffer, &config_copied_bytes);
  if (unlikely(config_cobhan_buffer == nullptr)) {
    LogErrorAndThrow(env, "setup", "Failed to copy config to cobhan buffer");
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
    LogErrorAndThrow(env, "setup", std::to_string(result));
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
    size_t ouput_cobhan_buffer_size =
        calculate_cobhan_buffer_size_bytes(asherah_output_size_bytes);
    debug_log_alloca("encrypt_to_json", "output_cobhan_buffer",
                     ouput_cobhan_buffer_size);
    output_cobhan_buffer = (char *)alloca(ouput_cobhan_buffer_size);
    configure_cbuffer(output_cobhan_buffer,
                      asherah_output_size_bytes + safety_padding_bytes);
  } else {
    output_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "output_cobhan_buffer", asherah_output_size_bytes);
    output_cobhan_buffer = output_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(output_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "encrypt_to_json",
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
    return LogErrorAndThrow(env, "encrypt_to_json", std::to_string(result));
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
    return LogErrorAndThrow(env, "encrypt", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(env, "encrypt", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsBuffer())) {
    return LogErrorAndThrow(env, "encrypt", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return LogErrorAndThrow(env, "encrypt",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return LogErrorAndThrow(env, "encrypt", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("encrypt", "partition_id_cobhan_buffer", partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer = (char *)alloca(partition_id_cobhan_buffer_size_bytes);
  } else {
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "encrypt",
                            "Failed to allocate partitionId cobhan buffer");
  }

  // Copy
  size_t partition_copied_bytes;
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "encrypt",
                            "Failed to copy partitionId to cobhan buffer");
  }

  // Determine size
  Napi::Buffer<unsigned char> input_napi_buffer =
      info[1].As<Napi::Buffer<unsigned char>>();
  size_t input_byte_length = input_napi_buffer.ByteLength();
  if (unlikely(input_byte_length == 0)) {
    return LogErrorAndThrow(env, "encrypt", "input is empty");
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_buffer_unique_ptr;
  if (input_byte_length < max_stack_alloc_size) {
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_byte_length);
    debug_log_alloca("encrypt", "input_cobhan_buffer",
                     input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
  } else {
    input_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_byte_length);
    input_cobhan_buffer = input_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(
        env, "encrypt", "Failed to allocate cobhan buffer for input buffer");
  }

  // Copy
  memcpy(input_cobhan_buffer + cobhan_header_size_bytes,
         input_napi_buffer.Data(), input_byte_length);
  configure_cbuffer(input_cobhan_buffer, input_byte_length);

  return encrypt_to_json(env, partition_copied_bytes, input_byte_length,
                         partition_id_cobhan_buffer, input_cobhan_buffer);
}

Napi::Value encrypt_string(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("encrypt_string", "called");
  }

  if (unlikely(setup_state == 0)) {
    return LogErrorAndThrow(env, "encrypt_string", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(env, "encrypt_string", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return LogErrorAndThrow(env, "encrypt_string", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return LogErrorAndThrow(env, "encrypt_string",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return LogErrorAndThrow(env, "encrypt_string", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("encrypt_string", "partition_id_cobhan_buffer",
                     partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer = (char *)alloca(partition_id_cobhan_buffer_size_bytes);
  } else {
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "encrypt_string",
                            "Failed to allocate partitionId cobhan buffer");
  }

  // Copy
  size_t partition_copied_bytes;
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "encrypt_string",
                            "Failed to copy partitionId to cobhan buffer");
  }

  // Determine size
  size_t input_utf8_byte_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_byte_length = nstring_utf8_byte_length(env, input);
  if (unlikely(input_utf8_byte_length == (size_t)(-1))) {
    return LogErrorAndThrow(env, "encrypt_string",
                            "Failed to get input utf8 length");
  }
  if (unlikely(input_utf8_byte_length == 0)) {
    return LogErrorAndThrow(env, "encrypt_string", "input is empty");
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("encrypt_string", "input_cobhan_buffer", input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
  } else {
    input_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_utf8_byte_length);
    input_cobhan_buffer = input_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "encrypt_string",
                            "Failed to allocate input cobhan buffer");
  }

  // Copy
  size_t input_copied_bytes;
  input_cobhan_buffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_byte_length,
                              input_cobhan_buffer, &input_copied_bytes);
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "encrypt_string",
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
    return LogErrorAndThrow(env, "decrypt", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(env, "decrypt", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return LogErrorAndThrow(env, "decrypt", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length, partition_copied_bytes;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return LogErrorAndThrow(env, "decrypt",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return LogErrorAndThrow(env, "decrypt", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("decrypt", "partition_id_cobhan_buffer",
                     partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer =
        (char *)alloca(partition_id_cobhan_buffer_size_bytes);
  } else {
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt",
                            "Failed to allocate partition_id cobhan buffer");
  }

  // Copy
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt",
                            "Failed to copy partition_id to cobhan buffer");
  }

  // Determine size
  size_t input_utf8_byte_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_byte_length = nstring_utf8_byte_length(env, input);
  if (unlikely(input_utf8_byte_length == (size_t)(-1))) {
    return LogErrorAndThrow(env, "decrypt", "Failed to get input utf8 length");
  }
  if (unlikely(input_utf8_byte_length == 0)) {
    return LogErrorAndThrow(env, "decrypt", "input is empty");
  }

  if (unlikely(verbose_flag)) {
    debug_log("decrypt",
              "input size " + std::to_string(input_utf8_byte_length));
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt", "input_cobhan_buffer",
                     input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
  } else {
    input_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_utf8_byte_length);
    input_cobhan_buffer = input_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt",
                            "Failed to allocate input cobhan buffer");
  }

  // Copy
  size_t input_copied_bytes;
  input_cobhan_buffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_byte_length,
                              input_cobhan_buffer, &input_copied_bytes);
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt",
                            "Failed to copy input to cobhan buffer");
  }

  char *output_cobhan_buffer;
  std::unique_ptr<char[]> output_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    size_t output_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt", "output_cobhan_buffer",
                     output_cobhan_buffer_size_bytes);
    output_cobhan_buffer = (char *)alloca(output_cobhan_buffer_size_bytes);
    configure_cbuffer(output_cobhan_buffer, input_utf8_byte_length);
  } else {
    output_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("output_cobhan_buffer", input_utf8_byte_length);
    output_cobhan_buffer = output_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(output_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt",
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
    return LogErrorAndThrow(env, "decrypt", std::to_string(result));
  }

  return cbuffer_to_nbuffer(env, output_cobhan_buffer);
}

Napi::Value decrypt_string(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(verbose_flag)) {
    debug_log("decrypt_string", "called");
  }

  if (unlikely(setup_state == 0)) {
    return LogErrorAndThrow(env, "decrypt_string", "setup() not called");
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(env, "decrypt_string", "Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return LogErrorAndThrow(env, "decrypt_string", "Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_byte_length;
  Napi::String partition_id = info[0].As<Napi::String>();
  partition_utf8_byte_length = nstring_utf8_byte_length(env, partition_id);
  if (unlikely(partition_utf8_byte_length == (size_t)(-1))) {
    return LogErrorAndThrow(env, "decrypt_string",
                            "Failed to get partition_id utf8 length");
  }
  if (unlikely(partition_utf8_byte_length == 0)) {
    return LogErrorAndThrow(env, "decrypt_string", "partition_id is empty");
  }

  // Allocate
  char *partition_id_cobhan_buffer;
  std::unique_ptr<char[]> partition_id_cobhan_buffer_unique_ptr;
  if (partition_utf8_byte_length < max_stack_alloc_size) {
    size_t partition_id_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(partition_utf8_byte_length);
    debug_log_alloca("decrypt_string", "partition_id_cobhan_buffer",
                     partition_id_cobhan_buffer_size_bytes);
    partition_id_cobhan_buffer =
        (char *)alloca(partition_id_cobhan_buffer_size_bytes);
  } else {
    partition_id_cobhan_buffer_unique_ptr = heap_allocate_cbuffer(
        "partition_id_cobhan_buffer", partition_utf8_byte_length);
    partition_id_cobhan_buffer = partition_id_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt_string",
                            "Failed to allocate partitionId cobhan buffer");
  }

  // Copy
  size_t partition_copied_bytes;
  partition_id_cobhan_buffer = copy_nstring_to_cbuffer(
      env, partition_id, partition_utf8_byte_length, partition_id_cobhan_buffer,
      &partition_copied_bytes);
  if (unlikely(partition_id_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt_string",
                            "Failed to copy partitionId to cobhan buffer");
  }

  // Determine size
  size_t input_utf8_byte_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_byte_length = nstring_utf8_byte_length(env, input);
  if (unlikely(input_utf8_byte_length == (size_t)(-1))) {
    return LogErrorAndThrow(env, "decrypt_string",
                            "Failed to get input utf8 length");
  }
  if (unlikely(input_utf8_byte_length == 0)) {
    return LogErrorAndThrow(env, "decrypt_string", "input is empty");
  }

  // Allocate
  char *input_cobhan_buffer;
  std::unique_ptr<char[]> input_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    size_t input_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt_string", "input_cobhan_buffer",
                     input_cobhan_buffer_size_bytes);
    input_cobhan_buffer = (char *)alloca(input_cobhan_buffer_size_bytes);
  } else {
    input_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("input_cobhan_buffer", input_utf8_byte_length);
    input_cobhan_buffer = input_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt_string",
                            "Failed to allocate input cobhan buffer");
  }

  // Copy
  size_t input_copied_bytes;
  input_cobhan_buffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_byte_length,
                              input_cobhan_buffer, &input_copied_bytes);
  if (unlikely(input_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt_string",
                            "Failed to copy input to cobhan buffer");
  }

  char *output_cobhan_buffer;
  std::unique_ptr<char[]> output_cobhan_buffer_unique_ptr;
  if (input_utf8_byte_length < max_stack_alloc_size) {
    size_t output_cobhan_buffer_size_bytes =
        calculate_cobhan_buffer_size_bytes(input_utf8_byte_length);
    debug_log_alloca("decrypt_string", "output_cobhan_buffer",
                     output_cobhan_buffer_size_bytes);
    output_cobhan_buffer = (char *)alloca(output_cobhan_buffer_size_bytes);
    configure_cbuffer(output_cobhan_buffer, input_utf8_byte_length);
  } else {
    output_cobhan_buffer_unique_ptr =
        heap_allocate_cbuffer("output_cobhan_buffer", input_utf8_byte_length);
    output_cobhan_buffer = output_cobhan_buffer_unique_ptr.get();
  }
  if (unlikely(output_cobhan_buffer == nullptr)) {
    return LogErrorAndThrow(env, "decrypt_string",
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
    return LogErrorAndThrow(env, "decrypt_string", std::to_string(result));
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
    LogErrorAndThrow(env, "set_max_stack_alloc_item_size",
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
    LogErrorAndThrow(env, "set_safety_padding_overhead",
                     "Wrong number of arguments");
    return;
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

NODE_API_MODULE(napiasherah, Init)
