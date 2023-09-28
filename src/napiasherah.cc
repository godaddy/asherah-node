#include <napi.h>
#include "../lib/libasherah.h"

const int EstimatedEncryptionOverhead = 48;
const int EstimatedEnvelopeOverhead = 185;
const double Base64Overhead = 1.34;
int EstimatedIntermediateKeyOverhead = 0;
const int header_size = 64 / 8;
const char* SetupJsonFailedMessage = "SetupJson failed: ";
const char* DecryptFailedMessage = "Decrypt failed: ";
const char* EncryptFailedMessage = "Encrypt failed: ";
const char* EncryptStringFailedMessage = "Encrypt output conversion to string failed";

void finalize_cbuffer(napi_env env, void* finalize_data) {
  unsigned char* buffer = ((unsigned char*) finalize_data) - header_size;
  delete[] buffer;
}

inline int estimate_buffer(int dataLen, int partitionLen) {
  double estimatedDataLen = double(dataLen + EstimatedEncryptionOverhead) * Base64Overhead;
  return int(EstimatedEnvelopeOverhead + EstimatedIntermediateKeyOverhead + partitionLen + estimatedDataLen);
}

//extern GoInt32 SetupJson(void* configJson);
Napi::Value Napi_SetupJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<unsigned char> configJson = info[0].As<Napi::Buffer<unsigned char>>();

  EstimatedIntermediateKeyOverhead = 4096; //TODO: config.ProductID.length + config.ServiceName.length

  GoInt32 result = SetupJson(configJson.Data());
  if (result < 0) {
      Napi::TypeError::New(env, SetupJsonFailedMessage + std::to_string(result))
        .ThrowAsJavaScriptException();
  }
  return env.Null();
}

//extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void* jsonPtr);
Napi::Value Napi_EncryptToJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsBuffer() || !info[1].IsBuffer()) {
    Napi::TypeError::New(env, "Wrong argument types")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<unsigned char> partitionId = info[0].As<Napi::Buffer<unsigned char>>();
  Napi::Buffer<unsigned char> data = info[1].As<Napi::Buffer<unsigned char>>();

  //Call estimate_buffer to determine Cobhan output buffer length
  int bufferSize = estimate_buffer(partitionId.Length(), data.Length());

  //C++ allocate the Cobhan output buffer with estimated length plus Cobhan buffer header size
  unsigned char *cobhanOutputBuffer = new unsigned char[bufferSize + header_size];

  //Initialize the Cobhan output buffer header
  *((int32_t*)cobhanOutputBuffer) = bufferSize;
  *((int32_t*)cobhanOutputBuffer+sizeof(int32_t)) = 0;

  GoInt32 result = EncryptToJson(partitionId.Data(), data.Data(), cobhanOutputBuffer);
  if (result < 0) {
      delete[] cobhanOutputBuffer;
      Napi::TypeError::New(env, EncryptFailedMessage + std::to_string(result))
        .ThrowAsJavaScriptException();
      return env.Null();
  }

  napi_value output;
  napi_status status = napi_create_string_utf8(env, ((const char*) cobhanOutputBuffer) + header_size, *((int*) cobhanOutputBuffer), &output);
  delete[] cobhanOutputBuffer;

  if(status != napi_ok) {
    Napi::TypeError::New(env, EncryptStringFailedMessage)
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::Value(env, output);
}

//extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void* dataPtr);
Napi::Value Napi_DecryptFromJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsBuffer() || !info[1].IsBuffer()) {
    Napi::TypeError::New(env, "Wrong argument types")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<unsigned char> partitionId = info[0].As<Napi::Buffer<unsigned char>>();
  Napi::Buffer<unsigned char> inputJson = info[1].As<Napi::Buffer<unsigned char>>();

  int bufferSize = inputJson.Length();

  //C++ allocate the Cobhan output buffer with estimated length plus Cobhan buffer header size
  unsigned char *cobhanOutputBuffer = new unsigned char[inputJson.Length() + header_size];

  //Initialize the Cobhan output buffer header
  *((int32_t*)cobhanOutputBuffer) = bufferSize;
  *((int32_t*)cobhanOutputBuffer+sizeof(int32_t)) = 0;

  //Allocate output buffer that is the same length as inputJson, plus the Cobhan buffer header
  //Initialize the Cobhan buffer header

  //NOTE: outputData is a Cobhan buffer, not just a buffer
  //Napi::Buffer<unsigned char> outputData = info[2].As<Napi::Buffer<unsigned char>>();

  GoInt32 result = DecryptFromJson(partitionId.Data(), inputJson.Data(), cobhanOutputBuffer);
  if (result < 0) {
      delete[] cobhanOutputBuffer;
      Napi::TypeError::New(env, DecryptFailedMessage + std::to_string(result))
        .ThrowAsJavaScriptException();
      return env.Null();
  }

  //Wrap the Cobhan output buffer with a NAPI buffer with finalizer
  return Napi::Buffer<unsigned char>::New(env,
    ((unsigned char*) cobhanOutputBuffer) + header_size,
    *((int*) cobhanOutputBuffer),
    &finalize_cbuffer);
}

//extern void Shutdown();
Napi::Value Napi_Shutdown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Shutdown();
  return env.Null();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "Napi_SetupJson"), Napi::Function::New(env, Napi_SetupJson));
  exports.Set(Napi::String::New(env, "Napi_EncryptToJson"), Napi::Function::New(env, Napi_EncryptToJson));
  exports.Set(Napi::String::New(env, "Napi_DecryptFromJson"), Napi::Function::New(env, Napi_DecryptFromJson));
  exports.Set(Napi::String::New(env, "Napi_Shutdown"), Napi::Function::New(env, Napi_Shutdown));
  return exports;
}

NODE_API_MODULE(napiasherah, Init)
