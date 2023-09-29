#include "../lib/libasherah.h"
#include <napi.h>

#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

const size_t est_encryption_overhead = 48;
const size_t est_envelope_overhead = 185;
const double base64_overhead = 1.34;
const size_t cobhan_header_size = 64 / 8;

size_t est_intermediate_key_overhead = 0;
size_t safety_padding_overhead = 0;
size_t max_stack_alloc_size = 1024;

const char *setupjson_failed_message = "SetupJson failed: ";
const char *decrypt_failed_message = "Decrypt failed: ";
const char *encrypt_failed_message = "Encrypt failed: ";

volatile int32_t setup_state = 0;

void finalize_wrapped_cbuffer(napi_env env, void *finalize_data) {
  char *buffer = ((char *)finalize_data) - cobhan_header_size;
  delete[] buffer;
}

__attribute__((always_inline)) inline size_t
estimate_buffer(size_t dataLen, size_t partitionLen) {
  double estimatedDataLen =
      double(dataLen + est_encryption_overhead) * base64_overhead;
  return size_t(est_envelope_overhead + est_intermediate_key_overhead +
                partitionLen + estimatedDataLen + safety_padding_overhead);
}

__attribute__((always_inline)) inline char *configure_cbuffer(char *buffer,
                                                              size_t length) {
  *((int32_t *)buffer) = length;
  *((int32_t *)(buffer + sizeof(int32_t))) = 0;
  return buffer;
}

__attribute__((always_inline)) inline char *allocate_cbuffer(size_t length) {
  char *cobhanBuffer = new (
      std::nothrow) char[length + cobhan_header_size + safety_padding_overhead];
  if (unlikely(cobhanBuffer == nullptr)) {
    return nullptr;
  }
  return configure_cbuffer(cobhanBuffer, length);
}

__attribute__((always_inline)) inline char *
nbuffer_to_cbuffer(Napi::Env &env, Napi::Buffer<unsigned char> &buffer) {
  size_t bufferLength = buffer.ByteLength();
  char *cobhanBuffer =
      new (std::nothrow) char[bufferLength + cobhan_header_size +
                              safety_padding_overhead];
  if (unlikely(cobhanBuffer == nullptr)) {
    return nullptr;
  }
  memcpy(cobhanBuffer + cobhan_header_size, buffer.Data(), bufferLength);
  return configure_cbuffer(cobhanBuffer, bufferLength);
}

__attribute__((always_inline)) inline Napi::Value
cbuffer_to_nstring(Napi::Env &env, char *cobhanBuffer) {
  napi_value output;
  // Using C function because it allows length delimited input
  napi_status status = napi_create_string_utf8(
      env, ((const char *)cobhanBuffer) + cobhan_header_size,
      *((int *)cobhanBuffer), &output);
  if (unlikely(status != napi_ok)) {
    Napi::Error::New(env,
                     "cbuffer_to_nstring failed: " + std::to_string(status))
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  return Napi::String(env, output);
}

__attribute__((always_inline)) inline size_t
nstring_utf8_length(Napi::Env &env, Napi::String &str) {
  napi_status status;
  size_t utf8_length;

  status = napi_get_value_string_utf8(env, str, nullptr, 0, &utf8_length);
  if (unlikely(status != napi_ok)) {
    Napi::Error::New(env, "nstring_to_cbuffer: Napi utf8 string conversion "
                          "failure (length check): " +
                              std::to_string(status))
        .ThrowAsJavaScriptException();
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
    Napi::Error::New(
        env, "nstring_to_cbuffer: Napi utf8 string conversion failure: " +
                 std::to_string(status))
        .ThrowAsJavaScriptException();
    return nullptr;
  }

  if (unlikely(copied_bytes != utf8_length)) {
    Napi::Error::New(
        env, "nstring_to_cbuffer: Did not copy expected number of bytes " +
                 std::to_string(utf8_length) + " copied " +
                 std::to_string(copied_bytes))
        .ThrowAsJavaScriptException();
    return nullptr;
  }

  *((int *)cbuffer) = copied_bytes;
  *((int *)(cbuffer + sizeof(int32_t))) = 0;

  if (length != nullptr)
    *length = copied_bytes;
  return cbuffer;
}

inline char *nstring_to_cbuffer(Napi::Env &env, Napi::String &str,
                                size_t *length = nullptr) {
  size_t utf8_length = nstring_utf8_length(env, str);
  if (unlikely(utf8_length == (size_t)(-1))) {
    return nullptr;
  }

  char *cobhanBuffer =
      new (std::nothrow) char[utf8_length + 1 + cobhan_header_size +
                              safety_padding_overhead];
  if (unlikely(cobhanBuffer == nullptr)) {
    return nullptr;
  }

  char *output =
      copy_nstring_to_cbuffer(env, str, utf8_length, cobhanBuffer, length);
  if (unlikely(output == nullptr)) {
    delete[] cobhanBuffer;
  }
  return output;
}

Napi::Value Napi_SetupJson(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(setup_state == 1)) {
    Napi::TypeError::New(env, encrypt_failed_message +
                                  std::string("SetupJson called twice"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

#ifdef COUNT_ARGUMENTS
  if (unlikely(info.Length() < 3)) {
    Napi::TypeError::New(env, "SetupJson: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

#ifdef CHECK_ARGUMENT_TYPES
  if (unlikely(!info[0].IsString() || !info[1].IsNumber() ||
               !info[2].IsNumber())) {
    Napi::TypeError::New(env, "SetupJson: Wrong argument types")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

  Napi::String configJson = info[0].As<Napi::String>();
  char *configJsonCobhanBuffer = nstring_to_cbuffer(env, configJson);
  if (unlikely(configJsonCobhanBuffer == nullptr)) {
    Napi::TypeError::New(
        env, setupjson_failed_message +
                 std::string(" failed to convert configJson to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Number productIdLength = info[1].As<Napi::Number>();
  Napi::Number serviceNameLength = info[2].As<Napi::Number>();

  est_intermediate_key_overhead =
      productIdLength.Int32Value() + serviceNameLength.Int32Value();

  // extern GoInt32 SetupJson(void* configJson);
  GoInt32 result = SetupJson(configJsonCobhanBuffer);
  delete[] configJsonCobhanBuffer;
  if (unlikely(result < 0)) {
    Napi::TypeError::New(env, setupjson_failed_message + std::to_string(result))
        .ThrowAsJavaScriptException();
  }
  setup_state = 1;
  return env.Null();
}

Napi::Value Napi_EncryptFromBufferToJson(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(setup_state == 0)) {
    Napi::TypeError::New(env, encrypt_failed_message +
                                  std::string("SetupJson not called"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

#ifdef COUNT_ARGUMENTS
  if (unlikely(info.Length() < 2)) {
    Napi::TypeError::New(env,
                         "EncryptFromBufferToJson: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

#ifdef CHECK_ARGUMENT_TYPES
  if (unlikely(!info[0].IsString() || !info[1].IsBuffer())) {
    Napi::TypeError::New(env, "EncryptFromBufferToJson: Wrong argument types")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

  size_t partition_utf8_length, partition_copied_bytes;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    Napi::TypeError::New(
        env, encrypt_failed_message +
                 std::string(" failed to get partitionId utf8 length"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  char *partitionIdCobhanBuffer;
  if (partition_utf8_length < max_stack_alloc_size) {
    partitionIdCobhanBuffer =
        (char *)alloca(partition_utf8_length + 1 + cobhan_header_size +
                       safety_padding_overhead);
  } else {
    partitionIdCobhanBuffer =
        new (std::nothrow) char[partition_utf8_length + 1 + cobhan_header_size +
                                safety_padding_overhead];
  }
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    Napi::TypeError::New(
        env, encrypt_failed_message +
                 std::string(" failed to allocate partitionId cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size) {
      delete[] partitionIdCobhanBuffer;
    }
    Napi::TypeError::New(
        env, encrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<unsigned char> data = info[1].As<Napi::Buffer<unsigned char>>();
  char *dataCobhanBuffer = nbuffer_to_cbuffer(env, data);
  if (unlikely(dataCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size) {
      delete[] partitionIdCobhanBuffer;
    }
    Napi::TypeError::New(
        env, encrypt_failed_message +
                 std::string(" failed to convert data buffer to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  size_t bufferSize =
      estimate_buffer(partition_copied_bytes, data.ByteLength());

  char *cobhanOutputBuffer;
  if (bufferSize < max_stack_alloc_size) {
    cobhanOutputBuffer =
        configure_cbuffer((char *)alloca(bufferSize + cobhan_header_size +
                                         safety_padding_overhead),
                          bufferSize);
  } else {
    cobhanOutputBuffer = allocate_cbuffer(bufferSize);
  }
  if (unlikely(cobhanOutputBuffer == nullptr)) {
    Napi::TypeError::New(
        env, encrypt_failed_message +
                 std::string(" failed to allocate cobhan output buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void*
  // jsonPtr);
  GoInt32 result = EncryptToJson(partitionIdCobhanBuffer, dataCobhanBuffer,
                                 cobhanOutputBuffer);
  if (partition_utf8_length >= max_stack_alloc_size)
    delete[] partitionIdCobhanBuffer;
  delete[] dataCobhanBuffer;
  if (unlikely(result < 0)) {
    if (bufferSize >= max_stack_alloc_size)
      delete[] cobhanOutputBuffer;
    Napi::TypeError::New(env, encrypt_failed_message + std::to_string(result))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Value output = cbuffer_to_nstring(env, cobhanOutputBuffer);
  if (bufferSize >= max_stack_alloc_size)
    delete[] cobhanOutputBuffer;
  return output;
}

Napi::Value Napi_EncryptFromStringToJson(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(setup_state == 0)) {
    Napi::TypeError::New(env, encrypt_failed_message +
                                  std::string("SetupJson not called"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

#ifdef COUNT_ARGUMENTS
  if (unlikely(info.Length() < 2)) {
    Napi::TypeError::New(env,
                         "EncryptFromStringToJson: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

#ifdef CHECK_ARGUMENT_TYPES
  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    Napi::TypeError::New(env, "EncryptFromStringToJson: Wrong argument types")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

  size_t partition_utf8_length, partition_copied_bytes;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    Napi::TypeError::New(
        env, encrypt_failed_message +
                 std::string(" failed to get partitionId utf8 length"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  char *partitionIdCobhanBuffer;
  if (partition_utf8_length < max_stack_alloc_size) {
    partitionIdCobhanBuffer =
        (char *)alloca(partition_utf8_length + 1 + cobhan_header_size +
                       safety_padding_overhead);
  } else {
    partitionIdCobhanBuffer =
        new (std::nothrow) char[partition_utf8_length + 1 + cobhan_header_size +
                                safety_padding_overhead];
  }
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size)
      delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(
        env, encrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  size_t input_utf8_length;
  Napi::String input = info[1].As<Napi::String>();
  char *inputCobhanBuffer = nstring_to_cbuffer(env, input, &input_utf8_length);
  if (unlikely(inputCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size)
      delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(
        env,
        encrypt_failed_message +
            std::string(" failed to convert input string to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // Call estimate_buffer to determine Cobhan output buffer length
  size_t bufferSize = estimate_buffer(partition_utf8_length, input_utf8_length);

  char *cobhanOutputBuffer;
  if (bufferSize < max_stack_alloc_size) {
    cobhanOutputBuffer =
        configure_cbuffer((char *)alloca(bufferSize + cobhan_header_size +
                                         safety_padding_overhead),
                          bufferSize);
  } else {
    cobhanOutputBuffer = allocate_cbuffer(bufferSize);
  }

  // extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void*
  // jsonPtr);
  GoInt32 result = EncryptToJson(partitionIdCobhanBuffer, inputCobhanBuffer,
                                 cobhanOutputBuffer);
  if (partition_utf8_length >= max_stack_alloc_size)
    delete[] partitionIdCobhanBuffer;
  delete[] inputCobhanBuffer;
  if (unlikely(result < 0)) {
    if (bufferSize >= max_stack_alloc_size)
      delete[] cobhanOutputBuffer;
    Napi::TypeError::New(env, encrypt_failed_message + std::to_string(result))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Value output = cbuffer_to_nstring(env, cobhanOutputBuffer);
  if (bufferSize >= max_stack_alloc_size)
    delete[] cobhanOutputBuffer;
  return output;
}

Napi::Value Napi_DecryptFromJsonToBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(setup_state == 0)) {
    Napi::TypeError::New(env, encrypt_failed_message +
                                  std::string("SetupJson not called"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

#ifdef COUNT_ARGUMENTS
  if (unlikely(info.Length() < 2)) {
    Napi::TypeError::New(env,
                         "DecryptFromJsonToBuffer: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

#ifdef CHECK_ARGUMENT_TYPES
  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    Napi::TypeError::New(env, "DecryptFromJsonToBuffer: Wrong argument types")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

  size_t partition_utf8_length, partition_copied_bytes;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    Napi::TypeError::New(
        env, decrypt_failed_message +
                 std::string(" failed to get partitionId utf8 length"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  char *partitionIdCobhanBuffer;
  if (partition_utf8_length < max_stack_alloc_size) {
    partitionIdCobhanBuffer =
        (char *)alloca(partition_utf8_length + 1 + cobhan_header_size +
                       safety_padding_overhead);
  } else {
    partitionIdCobhanBuffer =
        new (std::nothrow) char[partition_utf8_length + 1 + cobhan_header_size +
                                safety_padding_overhead];
  }
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size)
      delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(
        env, decrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  size_t utf8_length;
  Napi::String inputJson = info[1].As<Napi::String>();
  char *inputJsonCobhanBuffer =
      nstring_to_cbuffer(env, inputJson, &utf8_length);
  if (unlikely(inputJsonCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size)
      delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(
        env, decrypt_failed_message +
                 std::string(" failed to convert inputJson to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *cobhanOutputBuffer;
  if (utf8_length < max_stack_alloc_size) {
    cobhanOutputBuffer =
        configure_cbuffer((char *)alloca(utf8_length + cobhan_header_size +
                                         safety_padding_overhead),
                          utf8_length);
  } else {
    cobhanOutputBuffer = allocate_cbuffer(utf8_length);
  }
  if (unlikely(cobhanOutputBuffer == nullptr)) {
    Napi::TypeError::New(
        env, decrypt_failed_message +
                 std::string(" failed to allocate cobhan output buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partitionIdCobhanBuffer,
                                   inputJsonCobhanBuffer, cobhanOutputBuffer);
  if (partition_utf8_length >= max_stack_alloc_size)
    delete[] partitionIdCobhanBuffer;
  delete[] inputJsonCobhanBuffer;
  if (unlikely(result < 0)) {
    if (utf8_length >= max_stack_alloc_size) {
      delete[] cobhanOutputBuffer;
    }
    Napi::TypeError::New(env, decrypt_failed_message + std::to_string(result))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // Wrap the Cobhan output buffer with a NAPI buffer with finalizer
  if (utf8_length < max_stack_alloc_size) {
    return Napi::Buffer<unsigned char>::Copy(
        env, ((unsigned char *)cobhanOutputBuffer) + cobhan_header_size,
        *((int *)cobhanOutputBuffer));
  } else {
    return Napi::Buffer<unsigned char>::New(
        env, ((unsigned char *)cobhanOutputBuffer) + cobhan_header_size,
        *((int *)cobhanOutputBuffer), &finalize_wrapped_cbuffer);
  }
}

Napi::Value Napi_DecryptFromJsonToString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (unlikely(setup_state == 0)) {
    Napi::TypeError::New(env, encrypt_failed_message +
                                  std::string("SetupJson not called"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

#ifdef COUNT_ARGUMENTS
  if (unlikely(info.Length() < 2)) {
    Napi::TypeError::New(env,
                         "DecryptFromJsonToString: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

#ifdef CHECK_ARGUMENT_TYPES
  if (unlikely(!info[0].IsString() || !info[1].IsString())) {
    Napi::TypeError::New(env, "DecryptFromJsonToString: Wrong argument types")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

  size_t partition_utf8_length, partition_copied_bytes;
  Napi::String partitionId = info[0].As<Napi::String>();
  partition_utf8_length = nstring_utf8_length(env, partitionId);
  if (unlikely(partition_utf8_length == (size_t)(-1))) {
    return env.Null();
  }
  char *partitionIdCobhanBuffer;
  if (partition_utf8_length < max_stack_alloc_size) {
    partitionIdCobhanBuffer =
        (char *)alloca(partition_utf8_length + 1 + cobhan_header_size +
                       safety_padding_overhead);
  } else {
    partitionIdCobhanBuffer =
        new (std::nothrow) char[partition_utf8_length + 1 + cobhan_header_size +
                                safety_padding_overhead];
  }
  partitionIdCobhanBuffer =
      copy_nstring_to_cbuffer(env, partitionId, partition_utf8_length,
                              partitionIdCobhanBuffer, &partition_copied_bytes);
  if (unlikely(partitionIdCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size)
      delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(
        env, decrypt_failed_message +
                 std::string(" failed to copy partitionId to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  size_t utf8_length;
  Napi::String inputJson = info[1].As<Napi::String>();
  char *inputJsonCobhanBuffer =
      nstring_to_cbuffer(env, inputJson, &utf8_length);
  if (unlikely(inputJsonCobhanBuffer == nullptr)) {
    if (partition_utf8_length >= max_stack_alloc_size)
      delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(
        env, decrypt_failed_message +
                 std::string(" failed to convert inputJson to cobhan buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *cobhanOutputBuffer;
  if (utf8_length < max_stack_alloc_size) {
    cobhanOutputBuffer =
        configure_cbuffer((char *)alloca(utf8_length + cobhan_header_size +
                                         safety_padding_overhead),
                          utf8_length);
  } else {
    cobhanOutputBuffer = allocate_cbuffer(utf8_length);
  }
  if (unlikely(cobhanOutputBuffer == nullptr)) {
    Napi::TypeError::New(
        env, decrypt_failed_message +
                 std::string(" failed to allocate cobhan output buffer"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void*
  // dataPtr);
  GoInt32 result = DecryptFromJson(partitionIdCobhanBuffer,
                                   inputJsonCobhanBuffer, cobhanOutputBuffer);
  if (partition_utf8_length >= max_stack_alloc_size)
    delete[] partitionIdCobhanBuffer;
  delete[] inputJsonCobhanBuffer;
  if (unlikely(result < 0)) {
    if (utf8_length >= max_stack_alloc_size)
      delete[] cobhanOutputBuffer;
    Napi::TypeError::New(env, decrypt_failed_message + std::to_string(result))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Value output = cbuffer_to_nstring(env, cobhanOutputBuffer);
  if (utf8_length >= max_stack_alloc_size)
    delete[] cobhanOutputBuffer;
  return output;
}

Napi::Value Napi_Shutdown(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  setup_state = 0;
  // extern void Shutdown();
  Shutdown();
  return env.Null();
}

Napi::Value Napi_SetMaxStackAllocItemSize(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

#ifdef COUNT_ARGUMENTS
  if (unlikely(info.Length() < 1)) {
    Napi::TypeError::New(env,
                         "SetMaxStackAllocItemSize: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

  Napi::Number item_size = info[0].As<Napi::Number>();

  max_stack_alloc_size = (size_t)item_size.Int32Value();
  return env.Null();
}

Napi::Value Napi_SetSafetyPaddingOverhead(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

#ifdef COUNT_ARGUMENTS
  if (unlikely(info.Length() < 1)) {
    Napi::TypeError::New(env,
                         "SetSafetyPaddingOverhead: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
#endif

  Napi::Number safety_padding = info[0].As<Napi::Number>();

  safety_padding_overhead = (size_t)safety_padding.Int32Value();
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
  exports.Set(Napi::String::New(env, "Napi_Shutdown"),
              Napi::Function::New(env, Napi_Shutdown));
  return exports;
}

NODE_API_MODULE(napiasherah, Init)
