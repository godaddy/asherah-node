#include "../lib/libasherah.h"
#include <iostream>
#include <napi.h>

#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

const size_t cobhan_header_size = 64 / 8;
const size_t est_encryption_overhead = 48;
const size_t est_envelope_overhead = 185;
const double base64_overhead = 1.34;

size_t est_intermediate_key_overhead = 0;
size_t safety_padding = 0;
size_t max_stack_alloc_size = 2048;

const char *setupjson_failed_message = "SetupJson failed: ";
const char *decrypt_failed_message = "Decrypt failed: ";
const char *encrypt_failed_message = "Encrypt failed: ";

int32_t setup_state = 0;
int32_t verbose_flag = 0;

__attribute__((always_inline)) inline void debug_log(std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << message << std::endl << std::flush;
  }
}

__attribute__((always_inline)) inline void debug_log_alloca(size_t length) {
  if (unlikely(verbose_flag)) {
    std::cerr << "Calling alloca(" << length << ") (stack)" << std::endl << std::flush;
  }
}

__attribute__((always_inline)) inline void error_log(std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << message << std::endl << std::flush;
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
LogErrorAndThrow(Napi::Env &env, std::string error_msg) {
  error_log(error_msg);
  Napi::Error::New(env, error_msg).ThrowAsJavaScriptException();
  return env.Null();
}

__attribute__((always_inline)) inline size_t
cobhan_buffer_size(size_t dataLen) {
  return dataLen + cobhan_header_size + safety_padding +
         1; // Add one for possible NULL delimiter due to Node string functions
}

__attribute__((always_inline)) inline size_t
estimate_asherah_output_size(size_t dataLen, size_t partitionLen) {
  double estimatedDataLen =
      double(dataLen + est_encryption_overhead) * base64_overhead;
  return size_t(est_envelope_overhead + est_intermediate_key_overhead +
                partitionLen + estimatedDataLen + safety_padding);
}

__attribute__((always_inline)) inline void configure_cbuffer(char *buffer,
                                                             size_t length) {
  *((int32_t *)buffer) = length;
  // Reserved for future use
  *((int32_t *)(buffer + sizeof(int32_t))) = 0;
}

__attribute__((always_inline)) inline std::unique_ptr<char[]>
allocate_cbuffer(size_t length) {
  size_t cobhanBufferSize = cobhan_buffer_size(length);
  if (unlikely(verbose_flag)) {
    std::string log_msg =
        "allocate_cbuffer(" + std::to_string(length) +
        ") (heap) cobhanBufferSize: " + std::to_string(cobhanBufferSize);
    debug_log(log_msg);
  }

  char *cobhanBuffer = new (std::nothrow) char[cobhanBufferSize];
  if (unlikely(cobhanBuffer == nullptr)) {
    std::string error_msg = "allocate_cbuffer: new[" +
                            std::to_string(cobhanBufferSize) + " returned null";
    error_log(error_msg);
    return nullptr;
  }
  std::unique_ptr<char[]> cobhanBufferPtr(cobhanBuffer);
  configure_cbuffer(cobhanBuffer, length);
  return cobhanBufferPtr;
}

__attribute__((always_inline)) inline Napi::Value
cbuffer_to_nstring(Napi::Env &env, char *cobhanBuffer) {
  napi_value output;

  // Using C function because it allows length delimited input
  napi_status status = napi_create_string_utf8(
      env, ((const char *)cobhanBuffer) + cobhan_header_size,
      *((int *)cobhanBuffer), &output);

  if (unlikely(status != napi_ok)) {
    return LogErrorAndThrow(
        env, "cbuffer_to_nstring: napi_create_string_utf8 failed: " +
                 napi_status_to_string(status));
  }

  return Napi::String(env, output);
}

__attribute__((always_inline)) inline size_t
nstring_utf8_length(Napi::Env &env, Napi::String &str) {
  napi_status status;
  size_t utf8_length;

  status = napi_get_value_string_utf8(env, str, nullptr, 0, &utf8_length);
  if (unlikely(status != napi_ok)) {
    LogErrorAndThrow(
        env,
        "nstring_utf8_length: napi_get_value_string_utf8 length check failed" +
            napi_status_to_string(status));
    return (size_t)(-1);
  }

  return utf8_length;
}

__attribute__((always_inline)) inline char *
copy_nstring_to_cbuffer(Napi::Env &env, Napi::String &str, size_t utf8_length,
                        char *cbuffer, size_t *length = nullptr) {
  napi_status status;
  size_t copied_bytes;
  status = napi_get_value_string_utf8(env, str, cbuffer + cobhan_header_size,
                                      utf8_length + 1, &copied_bytes);
  if (unlikely(status != napi_ok)) {
    LogErrorAndThrow(env,
                     "copy_nstring_to_cbuffer: Napi utf8 string conversion "
                     "failure: " +
                         napi_status_to_string(status));
    return nullptr;
  }

  if (unlikely(copied_bytes != utf8_length)) {
    LogErrorAndThrow(env,
                     "copy_nstring_to_cbuffer: Did not copy expected number "
                     "of bytes " +
                         std::to_string(utf8_length) + " copied " +
                         std::to_string(copied_bytes));
    return nullptr;
  }

  *((int *)cbuffer) = copied_bytes;
  *((int *)(cbuffer + sizeof(int32_t))) = 0;

  if (length != nullptr)
    *length = copied_bytes;
  return cbuffer;
}

__attribute__((always_inline)) inline Napi::Buffer<unsigned char>
cbuffer_to_nbuffer(Napi::Env &env, char *cobhanBuffer) {
  return Napi::Buffer<unsigned char>::Copy(
      env, ((unsigned char *)cobhanBuffer) + cobhan_header_size,
      *((int *)cobhanBuffer));
}

Napi::Value Napi_SetupJson(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  debug_log("SetupJson called");

  if (unlikely(setup_state == 1)) {
    return LogErrorAndThrow(env, setupjson_failed_message +
                                     std::string("SetupJson called twice"));
  }

  if (unlikely(info.Length() < 3)) {
    return LogErrorAndThrow(env, "SetupJson: Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsNumber() ||
               !info[2].IsNumber() || !info[3].IsBoolean())) {
    return LogErrorAndThrow(env, "SetupJson: Wrong argument types");
  }

  Napi::Boolean verbose = info[3].As<Napi::Boolean>();
  verbose.Value() ? verbose_flag = 1 : verbose_flag = 0;
  debug_log("SetupJson: verbose_flag: " + std::to_string(verbose_flag) +
            " verbose.Value(): " + std::to_string(verbose.Value()));

  // Determine size
  size_t config_utf8_length;
  Napi::String configJson = info[0].As<Napi::String>();
  config_utf8_length = nstring_utf8_length(env, configJson);
  if (unlikely(config_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to get configJson utf8 length"));
  }

  // Allocate
  char *configJsonCobhanBuffer;
  std::unique_ptr<char[]> configJsonCobhanBufferPtr;
  if (config_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(config_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    configJsonCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    configJsonCobhanBufferPtr = allocate_cbuffer(config_utf8_length);
    configJsonCobhanBuffer = configJsonCobhanBufferPtr.get();
  }
  if (unlikely(configJsonCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to allocate configJson cobhan buffer"));
  }

  // Copy
  size_t config_copied_bytes;
  configJsonCobhanBuffer =
      copy_nstring_to_cbuffer(env, configJson, config_utf8_length,
                              configJsonCobhanBuffer, &config_copied_bytes);
  if (unlikely(configJsonCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to copy configJson to cobhan buffer"));
  }

  Napi::Number productIdLength = info[1].As<Napi::Number>();
  Napi::Number serviceNameLength = info[2].As<Napi::Number>();

  est_intermediate_key_overhead =
      productIdLength.Int32Value() + serviceNameLength.Int32Value();

  // extern GoInt32 SetupJson(void* configJson);
  GoInt32 result = SetupJson(configJsonCobhanBuffer);
  if (unlikely(result < 0)) {
    return LogErrorAndThrow(env,
                            setupjson_failed_message + std::to_string(result));
  }
  setup_state = 1;
  return env.Null();
}

Napi::Value encrypt_to_json(Napi::Env &env, size_t partition_bytes,
                            size_t data_bytes, char *partitionIdCobhanBuffer,
                            char *dataCobhanBuffer) {

  size_t asherah_output_size =
      estimate_asherah_output_size(partition_bytes, data_bytes);

  char *cobhanOutputBuffer;
  std::unique_ptr<char[]> cobhanOutputBufferPtr;
  if (asherah_output_size < max_stack_alloc_size) {
    size_t cobhan_ouput_buffer_size = cobhan_buffer_size(asherah_output_size);
    debug_log_alloca(cobhan_ouput_buffer_size);
    cobhanOutputBuffer = (char *)alloca(cobhan_ouput_buffer_size);
    configure_cbuffer(cobhanOutputBuffer, asherah_output_size);
  } else {
    cobhanOutputBufferPtr = allocate_cbuffer(asherah_output_size);
    cobhanOutputBuffer = cobhanOutputBufferPtr.get();
  }
  if (unlikely(cobhanOutputBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to allocate cobhan output buffer"));
  }

  // extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void*
  // jsonPtr);
  GoInt32 result = EncryptToJson(partitionIdCobhanBuffer, dataCobhanBuffer,
                                 cobhanOutputBuffer);
  if (unlikely(result < 0)) {
    return LogErrorAndThrow(env,
                            encrypt_failed_message + std::to_string(result));
  }

  Napi::Value output = cbuffer_to_nstring(env, cobhanOutputBuffer);
  return output;
}

Napi::Value Napi_EncryptFromBufferToJson(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  debug_log("EncryptFromBufferToJson called");

  if (unlikely(setup_state == 0)) {
    return LogErrorAndThrow(env, encrypt_failed_message +
                                     std::string("SetupJson not called"));
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(
        env, "EncryptFromBufferToJson: Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsBuffer())) {
    return LogErrorAndThrow(env,
                            "EncryptFromBufferToJson: Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_length;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to get partitionId utf8 length"));
  }

  // Allocate
  char *partitionIdCobhanBuffer;
  std::unique_ptr<char[]> partitionIdCobhanBufferPtr;
  if (partition_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(partition_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    partitionIdCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    partitionIdCobhanBufferPtr = allocate_cbuffer(partition_utf8_length);
    partitionIdCobhanBuffer = partitionIdCobhanBufferPtr.get();
  }
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to allocate partitionId cobhan buffer"));
  }

  // Copy
  size_t partition_copied_bytes;
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"));
  }

  // Determine size
  Napi::Buffer<unsigned char> inputNapiBuffer =
      info[1].As<Napi::Buffer<unsigned char>>();
  size_t input_byte_length = inputNapiBuffer.ByteLength();

  // Allocate
  char *inputBuffer;
  std::unique_ptr<char[]> inputBufferPtr;
  if (input_byte_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(input_byte_length);
    debug_log_alloca(cobhan_buf_size);
    inputBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    inputBufferPtr = allocate_cbuffer(input_byte_length);
    inputBuffer = inputBufferPtr.get();
  }
  if (unlikely(inputBuffer == nullptr)) {
    return LogErrorAndThrow(
        env,
        encrypt_failed_message +
            std::string(" failed to allocate cobhan buffer for input buffer"));
  }

  // Copy
  memcpy(inputBuffer + cobhan_header_size, inputNapiBuffer.Data(),
         input_byte_length);
  configure_cbuffer(inputBuffer, input_byte_length);

  return encrypt_to_json(env, partition_copied_bytes, input_byte_length,
                         partitionIdCobhanBuffer, inputBuffer);
}

Napi::Value Napi_EncryptFromStringToJson(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  debug_log("EncryptFromStringToJson called\n\n");

  if (unlikely(setup_state == 0)) {
    return LogErrorAndThrow(env, encrypt_failed_message +
                                     std::string("SetupJson not called"));
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(
        env, "EncryptFromStringToJson: Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return LogErrorAndThrow(env,
                            "EncryptFromStringToJson: Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_length;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to get partitionId utf8 length"));
  }

  // Allocate
  char *partitionIdCobhanBuffer;
  std::unique_ptr<char[]> partitionIdCobhanBufferPtr;
  if (partition_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(partition_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    partitionIdCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    partitionIdCobhanBufferPtr = allocate_cbuffer(partition_utf8_length);
    partitionIdCobhanBuffer = partitionIdCobhanBufferPtr.get();
  }
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to allocate partitionId cobhan buffer"));
  }

  // Copy
  size_t partition_copied_bytes;
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"));
  }

  // Determine size
  size_t input_utf8_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_length = nstring_utf8_length(env, input);
  if (unlikely(input_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to get input utf8 length"));
  }

  // Allocate
  char *inputCobhanBuffer;
  std::unique_ptr<char[]> inputCobhanBufferPtr;
  if (input_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(input_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    inputCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    inputCobhanBufferPtr = allocate_cbuffer(input_utf8_length);
    inputCobhanBuffer = inputCobhanBufferPtr.get();
  }
  if (unlikely(inputCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to allocate input cobhan buffer"));
  }

  // Copy
  size_t input_copied_bytes;
  inputCobhanBuffer = copy_nstring_to_cbuffer(
      env, input, input_utf8_length, inputCobhanBuffer, &input_copied_bytes);
  if (unlikely(inputCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, encrypt_failed_message +
                 std::string(" failed to copy input to cobhan buffer"));
  }

  return encrypt_to_json(env, partition_copied_bytes, input_utf8_length,
                         partitionIdCobhanBuffer, inputCobhanBuffer);
}

Napi::Value Napi_DecryptFromJsonToBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  debug_log("DecryptFromJsonToBuffer called\n\n");

  if (unlikely(setup_state == 0)) {
    return LogErrorAndThrow(env, decrypt_failed_message +
                                     std::string("SetupJson not called"));
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(
        env, "DecryptFromJsonToBuffer: Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return LogErrorAndThrow(env,
                            "DecryptFromJsonToBuffer: Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_length, partition_copied_bytes;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to get partitionId utf8 length"));
  }

  // Allocate
  char *partitionIdCobhanBuffer;
  std::unique_ptr<char[]> partitionIdCobhanBufferPtr;
  if (partition_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(partition_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    partitionIdCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    partitionIdCobhanBufferPtr = allocate_cbuffer(partition_utf8_length);
    partitionIdCobhanBuffer = partitionIdCobhanBufferPtr.get();
  }
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to allocate partitionId cobhan buffer"));
  }

  // Copy
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"));
  }

  // Determine size
  size_t input_utf8_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_length = nstring_utf8_length(env, input);
  if (unlikely(input_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to get input utf8 length"));
  }
  debug_log("DecryptFromJsonToBuffer input size " +
            std::to_string(input_utf8_length));

  // Allocate
  char *inputJsonCobhanBuffer;
  std::unique_ptr<char[]> inputJsonCobhanBufferPtr;
  if (input_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(input_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    inputJsonCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    inputJsonCobhanBufferPtr = allocate_cbuffer(input_utf8_length);
    inputJsonCobhanBuffer = inputJsonCobhanBufferPtr.get();
  }
  if (unlikely(inputJsonCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to allocate input cobhan buffer"));
  }

  // Copy
  size_t input_copied_bytes;
  inputJsonCobhanBuffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_length,
                              inputJsonCobhanBuffer, &input_copied_bytes);
  if (unlikely(inputJsonCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to copy inputJson to cobhan buffer"));
  }

  char *cobhanOutputBuffer;
  std::unique_ptr<char[]> cobhanOutputBufferPtr;
  if (input_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(input_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    cobhanOutputBuffer = (char *)alloca(cobhan_buf_size);
    configure_cbuffer(cobhanOutputBuffer, input_utf8_length);
  } else {
    cobhanOutputBufferPtr = allocate_cbuffer(input_utf8_length);
    cobhanOutputBuffer = cobhanOutputBufferPtr.get();
  }
  if (unlikely(cobhanOutputBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to allocate cobhan output buffer"));
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partitionIdCobhanBuffer,
                                   inputJsonCobhanBuffer, cobhanOutputBuffer);
  if (unlikely(result < 0)) {
    return LogErrorAndThrow(env,
                            decrypt_failed_message + std::to_string(result));
  }

  return cbuffer_to_nbuffer(env, cobhanOutputBuffer);
}

Napi::Value Napi_DecryptFromJsonToString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  debug_log("DecryptFromJsonToString called\n\n");

  if (unlikely(setup_state == 0)) {
    return LogErrorAndThrow(env, decrypt_failed_message +
                                     std::string("SetupJson not called"));
  }

  if (unlikely(info.Length() < 2)) {
    return LogErrorAndThrow(
        env, "DecryptFromJsonToString: Wrong number of arguments");
  }

  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    return LogErrorAndThrow(env,
                            "DecryptFromJsonToString: Wrong argument types");
  }

  // Determine size
  size_t partition_utf8_length;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to get partitionId utf8 length"));
  }

  // Allocate
  char *partitionIdCobhanBuffer;
  std::unique_ptr<char[]> partitionIdCobhanBufferPtr;
  if (partition_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(partition_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    partitionIdCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    partitionIdCobhanBufferPtr = allocate_cbuffer(partition_utf8_length);
    partitionIdCobhanBuffer = partitionIdCobhanBufferPtr.get();
  }
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to allocate partitionId cobhan buffer"));
  }

  // Copy
  size_t partition_copied_bytes;
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"));
  }

  // Determine size
  size_t input_utf8_length;
  Napi::String input = info[1].As<Napi::String>();
  input_utf8_length = nstring_utf8_length(env, input);
  if (unlikely(input_utf8_length == (size_t)(-1))) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to get input utf8 length"));
  }

  // Allocate
  char *inputJsonCobhanBuffer;
  std::unique_ptr<char[]> inputJsonCobhanBufferPtr;
  if (input_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(input_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    inputJsonCobhanBuffer = (char *)alloca(cobhan_buf_size);
  } else {
    inputJsonCobhanBufferPtr = allocate_cbuffer(input_utf8_length);
    inputJsonCobhanBuffer = inputJsonCobhanBufferPtr.get();
  }
  if (unlikely(inputJsonCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to allocate input cobhan buffer"));
  }

  // Copy
  size_t input_copied_bytes;
  inputJsonCobhanBuffer =
      copy_nstring_to_cbuffer(env, input, input_utf8_length,
                              inputJsonCobhanBuffer, &input_copied_bytes);
  if (unlikely(inputJsonCobhanBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to copy inputJson to cobhan buffer"));
  }

  char *cobhanOutputBuffer;
  std::unique_ptr<char[]> cobhanOutputBufferPtr;
  if (input_utf8_length < max_stack_alloc_size) {
    size_t cobhan_buf_size = cobhan_buffer_size(input_utf8_length);
    debug_log_alloca(cobhan_buf_size);
    cobhanOutputBuffer = (char *)alloca(cobhan_buf_size);
    configure_cbuffer(cobhanOutputBuffer, input_utf8_length);
  } else {
    cobhanOutputBufferPtr = allocate_cbuffer(input_utf8_length);
    cobhanOutputBuffer = cobhanOutputBufferPtr.get();
  }
  if (unlikely(cobhanOutputBuffer == nullptr)) {
    return LogErrorAndThrow(
        env, decrypt_failed_message +
                 std::string(" failed to allocate cobhan output buffer"));
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partitionIdCobhanBuffer,
                                   inputJsonCobhanBuffer, cobhanOutputBuffer);
  if (unlikely(result < 0)) {
    return LogErrorAndThrow(env,
                            decrypt_failed_message + std::to_string(result));
  }

  Napi::Value output = cbuffer_to_nstring(env, cobhanOutputBuffer);
  return output;
}

Napi::Value Napi_Shutdown(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  debug_log("Shutdown called");

  setup_state = 0;
  // extern void Shutdown();
  Shutdown();
  return env.Null();
}

Napi::Value Napi_SetMaxStackAllocItemSize(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  debug_log("SetMaxStackAllocItemSize called");

  if (unlikely(info.Length() < 1)) {
    return LogErrorAndThrow(
        env, "SetMaxStackAllocItemSize: Wrong number of arguments");
  }

  Napi::Number item_size = info[0].As<Napi::Number>();

  max_stack_alloc_size = (size_t)item_size.Int32Value();
  return env.Null();
}

Napi::Value Napi_SetSafetyPaddingOverhead(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  debug_log("SetSafetyPaddingOverhead called");

  if (unlikely(info.Length() < 1)) {
    return LogErrorAndThrow(
        env, "SetSafetyPaddingOverhead: Wrong number of arguments");
  }

  Napi::Number safety_padding_number = info[0].As<Napi::Number>();

  safety_padding = (size_t)safety_padding_number.Int32Value();
  return env.Null();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "Napi_SetupJson"),
              Napi::Function::New(env, Napi_SetupJson));
  exports.Set(Napi::String::New(env, "Napi_EncryptFromBufferToJson"),
              Napi::Function::New(env, Napi_EncryptFromBufferToJson));
  exports.Set(Napi::String::New(env, "Napi_EncryptFromStringToJson"),
              Napi::Function::New(env, Napi_EncryptFromStringToJson));
  exports.Set(Napi::String::New(env, "Napi_DecryptFromJsonToBuffer"),
              Napi::Function::New(env, Napi_DecryptFromJsonToBuffer));
  exports.Set(Napi::String::New(env, "Napi_DecryptFromJsonToString"),
              Napi::Function::New(env, Napi_DecryptFromJsonToString));
  exports.Set(Napi::String::New(env, "Napi_SetSafetyPaddingOverhead"),
              Napi::Function::New(env, Napi_SetSafetyPaddingOverhead));
  exports.Set(Napi::String::New(env, "Napi_SetMaxStackAllocItemSize"),
              Napi::Function::New(env, Napi_SetMaxStackAllocItemSize));
  exports.Set(Napi::String::New(env, "Napi_Shutdown"),
              Napi::Function::New(env, Napi_Shutdown));
  return exports;
}

NODE_API_MODULE(napiasherah, Init)
