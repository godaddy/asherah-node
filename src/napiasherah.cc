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

Napi::Value Napi_SetupJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 4) {
    Napi::TypeError::New(env, "SetupJson: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString() || !info[1].IsNumber() || !info[2].IsNumber() || !info[3].IsNumber()) {
    Napi::TypeError::New(env, "SetupJson: Wrong argument types")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  int32_t configStrLen = info[3].As<Napi::Number>().Int32Value();

  napi_status status;
  size_t utf8_length, copied_bytes;
  Napi::String configJson = info[0].As<Napi::String>();
  status = napi_get_value_string_utf8(env, configJson, NULL, 0, &utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "SetupJson: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *configJsonCobhanBuffer = new char[utf8_length + 1 + header_size];

  status = napi_get_value_string_utf8(env, configJson, configJsonCobhanBuffer + header_size, utf8_length + 1, &copied_bytes);
  if(copied_bytes != utf8_length || copied_bytes != configStrLen) {
    delete[] configJsonCobhanBuffer;
    Napi::TypeError::New(env, "SetupJson: Did not copy expected number of bytes " + std::to_string(utf8_length) + " copied " + std::to_string(copied_bytes) + " JS " + std::to_string(configStrLen))
      .ThrowAsJavaScriptException();
    return env.Null();
  }
  if(status != napi_ok) {
    delete[] configJsonCobhanBuffer;
    Napi::TypeError::New(env, "SetupJson: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)configJsonCobhanBuffer) = copied_bytes;
  *((int*)(configJsonCobhanBuffer+sizeof(int32_t))) = 0;

  Napi::Number productIdLength = info[1].As<Napi::Number>();
  Napi::Number serviceNameLength = info[2].As<Napi::Number>();

  EstimatedIntermediateKeyOverhead = productIdLength.Int32Value() + serviceNameLength.Int32Value();

  //extern GoInt32 SetupJson(void* configJson);
  GoInt32 result = SetupJson(configJsonCobhanBuffer);
  delete[] configJsonCobhanBuffer;
  if (result < 0) {
      Napi::TypeError::New(env, SetupJsonFailedMessage + std::to_string(result))
        .ThrowAsJavaScriptException();
  }
  return env.Null();
}

Napi::Value Napi_EncryptFromBufferToJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "EncryptFromBufferToJson: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString() || !info[1].IsBuffer()) {
    Napi::TypeError::New(env, "EncryptFromBufferToJson: Wrong argument types")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  napi_status status;
  size_t utf8_length, copied_bytes;

  Napi::String partitionId = info[0].As<Napi::String>();

  status = napi_get_value_string_utf8(env, partitionId, NULL, 0, &utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "EncryptFromBufferToJson: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *partitionIdCobhanBuffer = new char[utf8_length + 1 + header_size + 16384];

  status = napi_get_value_string_utf8(env, partitionId, partitionIdCobhanBuffer + header_size, utf8_length + 1 + 16384, &copied_bytes);
  if(copied_bytes != utf8_length) {
    delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(env, "EncryptFromStringToJson: partitionId did not copy expected number of bytes " + std::to_string(utf8_length) + " copied " + std::to_string(copied_bytes))
      .ThrowAsJavaScriptException();
    return env.Null();
  }
  if(status != napi_ok) {
    Napi::TypeError::New(env, "EncryptFromBufferToJson: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)partitionIdCobhanBuffer) = copied_bytes;
  *((int*)(partitionIdCobhanBuffer+sizeof(int32_t))) = 0;

  //TODO: Convert data from Buffer to Cobhan buffer
  Napi::Buffer<unsigned char> data = info[1].As<Napi::Buffer<unsigned char>>();
  size_t dataBufferLength = data.ByteLength();
  char *dataCobhanBuffer = new char[dataBufferLength + header_size];

  memcpy(dataCobhanBuffer + header_size, data.Data(), dataBufferLength);
  *((int*)dataCobhanBuffer) = dataBufferLength;
  *((int*)(dataCobhanBuffer+sizeof(int32_t))) = 0;

  //Call estimate_buffer to determine Cobhan output buffer length
  int bufferSize = estimate_buffer(utf8_length, dataBufferLength);

  //C++ allocate the Cobhan output buffer with estimated length plus Cobhan buffer header size
  unsigned char *cobhanOutputBuffer = new unsigned char[bufferSize + header_size];

  //Initialize the Cobhan output buffer header
  *((int32_t*)cobhanOutputBuffer) = bufferSize;
  *((int32_t*)(cobhanOutputBuffer+sizeof(int32_t))) = 0;

  //extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void* jsonPtr);
  GoInt32 result = EncryptToJson(partitionIdCobhanBuffer, dataCobhanBuffer, cobhanOutputBuffer);
  if (result < 0) {
      delete[] cobhanOutputBuffer;
      Napi::TypeError::New(env, EncryptFailedMessage + std::to_string(result))
        .ThrowAsJavaScriptException();
      return env.Null();
  }

  napi_value output;
  //Using C function because it allows length delimited input
  status = napi_create_string_utf8(env, ((const char*) cobhanOutputBuffer) + header_size, *((int*) cobhanOutputBuffer), &output);
  delete[] cobhanOutputBuffer;

  if(status != napi_ok) {
    Napi::TypeError::New(env, EncryptStringFailedMessage)
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::Value(env, output);
}

Napi::Value Napi_EncryptFromStringToJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "EncryptFromStringToJson: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString() || !info[1].IsString()) {
    Napi::TypeError::New(env, "EncryptFromStringToJson: Wrong argument types")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  napi_status status;
  size_t partition_utf8_length, input_utf8_length, copied_bytes;

  Napi::String partitionId = info[0].As<Napi::String>();

  status = napi_get_value_string_utf8(env, partitionId, NULL, 0, &partition_utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "EncryptFromStringToJson: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *partitionIdCobhanBuffer = new char[partition_utf8_length + 1 + header_size];

  status = napi_get_value_string_utf8(env, partitionId, partitionIdCobhanBuffer + header_size, partition_utf8_length + 1, &copied_bytes);
  if(copied_bytes != partition_utf8_length) {
    delete[] partitionIdCobhanBuffer;
    Napi::TypeError::New(env, "EncryptFromStringToJson: partitionId did not copy expected number of bytes " + std::to_string(partition_utf8_length) + " copied " + std::to_string(copied_bytes))
      .ThrowAsJavaScriptException();
    return env.Null();
  }
  if(status != napi_ok) {
    Napi::TypeError::New(env, "EncryptFromStringToJson: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)partitionIdCobhanBuffer) = copied_bytes;
  *((int*)(partitionIdCobhanBuffer+sizeof(int32_t))) = 0;

  Napi::String input = info[1].As<Napi::String>();

  status = napi_get_value_string_utf8(env, input, NULL, 0, &input_utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "EncryptFromStringToJson: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *inputCobhanBuffer = new char[input_utf8_length + 1 + header_size];

  status = napi_get_value_string_utf8(env, input, inputCobhanBuffer + header_size, input_utf8_length + 1, &copied_bytes);
  if(copied_bytes != input_utf8_length) {
    delete[] inputCobhanBuffer;
    Napi::TypeError::New(env, "EncryptFromStringToJson: input did not copy expected number of bytes " + std::to_string(input_utf8_length) + " copied " + std::to_string(copied_bytes))
      .ThrowAsJavaScriptException();
    return env.Null();
  }
  if(status != napi_ok) {
    delete[] inputCobhanBuffer;
    Napi::TypeError::New(env, "EncryptFromStringToJson: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)inputCobhanBuffer) = copied_bytes;
  *((int*)(inputCobhanBuffer+sizeof(int32_t))) = 0;

  //Call estimate_buffer to determine Cobhan output buffer length
  int bufferSize = estimate_buffer(partition_utf8_length, input_utf8_length);

  //C++ allocate the Cobhan output buffer with estimated length plus Cobhan buffer header size
  unsigned char *cobhanOutputBuffer = new unsigned char[bufferSize + header_size + 16384];

  //Initialize the Cobhan output buffer header
  *((int32_t*)cobhanOutputBuffer) = bufferSize + 16384;
  *((int32_t*)(cobhanOutputBuffer+sizeof(int32_t))) = 0;

  //extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void* jsonPtr);
  GoInt32 result = EncryptToJson(partitionIdCobhanBuffer, inputCobhanBuffer, cobhanOutputBuffer);
  delete[] partitionIdCobhanBuffer;
  delete[] inputCobhanBuffer;
  if (result < 0) {
      delete[] cobhanOutputBuffer;
      Napi::TypeError::New(env, EncryptFailedMessage + std::to_string(result))
        .ThrowAsJavaScriptException();
      return env.Null();
  }

  napi_value output;
  //Using C function because it allows length delimited input
  status = napi_create_string_utf8(env, ((const char*) cobhanOutputBuffer) + header_size, *((int*) cobhanOutputBuffer), &output);
  //delete[] cobhanOutputBuffer;

  if(status != napi_ok) {
    Napi::TypeError::New(env, EncryptStringFailedMessage)
      .ThrowAsJavaScriptException();
    return env.Null();
  }
  /*
    Napi::TypeError::New(env, std::string((char*)cobhanOutputBuffer + header_size, 0, *((int*) cobhanOutputBuffer)))
      .ThrowAsJavaScriptException();
*/
  return Napi::Value(env, output);
}

Napi::Value Napi_DecryptFromJsonToBuffer(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "DecryptFromJsonToBuffer: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString() || !info[1].IsString()) {
    Napi::TypeError::New(env, "DecryptFromJsonToBuffer: Wrong argument types")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  napi_status status;
  size_t utf8_length, copied_bytes;

  Napi::String partitionId = info[0].As<Napi::String>();

  status = napi_get_value_string_utf8(env, partitionId, NULL, 0, &utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToBuffer: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *partitionIdCobhanBuffer = new char[utf8_length + 1 + header_size];

  status = napi_get_value_string_utf8(env, partitionId, partitionIdCobhanBuffer + header_size, utf8_length + 1, &copied_bytes);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToBuffer: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)partitionIdCobhanBuffer) = copied_bytes;
  *((int*)(partitionIdCobhanBuffer+sizeof(int32_t))) = 0;

  Napi::String inputJson = info[1].As<Napi::String>();

  status = napi_get_value_string_utf8(env, inputJson, NULL, 0, &utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToBuffer: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *inputJsonCobhanBuffer = new char[utf8_length + 1 + header_size];

  status = napi_get_value_string_utf8(env, inputJson, inputJsonCobhanBuffer + header_size, utf8_length + 1, &copied_bytes);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToBuffer: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)inputJsonCobhanBuffer) = copied_bytes;
  *((int*)(inputJsonCobhanBuffer+sizeof(int32_t))) = 0;

  int bufferSize = utf8_length;

  //C++ allocate the Cobhan output buffer with estimated length plus Cobhan buffer header size
  unsigned char *cobhanOutputBuffer = new unsigned char[utf8_length + header_size];

  //Initialize the Cobhan output buffer header
  *((int32_t*)cobhanOutputBuffer) = bufferSize;
  *((int32_t*)(cobhanOutputBuffer+sizeof(int32_t))) = 0;

  //extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void* dataPtr);
  GoInt32 result = DecryptFromJson(partitionIdCobhanBuffer, inputJsonCobhanBuffer, cobhanOutputBuffer);
  delete[] partitionIdCobhanBuffer;
  delete[] inputJsonCobhanBuffer;
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

Napi::Value Napi_DecryptFromJsonToString(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "DecryptFromJsonToString: Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString() || !info[1].IsString()) {
    Napi::TypeError::New(env, "DecryptFromJsonToString: Wrong argument types")
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  napi_status status;
  size_t utf8_length, copied_bytes;

  Napi::String partitionId = info[0].As<Napi::String>();

  status = napi_get_value_string_utf8(env, partitionId, NULL, 0, &utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToString: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *partitionIdCobhanBuffer = new char[utf8_length + 1 + header_size];

  status = napi_get_value_string_utf8(env, partitionId, partitionIdCobhanBuffer + header_size, utf8_length + 1, &copied_bytes);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToString: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)partitionIdCobhanBuffer) = copied_bytes;
  *((int*)(partitionIdCobhanBuffer+sizeof(int32_t))) = 0;

  Napi::String inputJson = info[1].As<Napi::String>();

  status = napi_get_value_string_utf8(env, inputJson, NULL, 0, &utf8_length);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToString: Napi utf8 string conversion failure (length check): " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  char *inputJsonCobhanBuffer = new char[utf8_length + 1 + header_size];

  status = napi_get_value_string_utf8(env, inputJson, inputJsonCobhanBuffer + header_size, utf8_length + 1, &copied_bytes);
  if(status != napi_ok) {
    Napi::TypeError::New(env, "DecryptFromJsonToString: Napi utf8 string conversion failure: " + std::to_string(status))
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  *((int*)inputJsonCobhanBuffer) = copied_bytes;
  *((int*)(inputJsonCobhanBuffer+sizeof(int32_t))) = 0;

  int bufferSize = utf8_length;

  //C++ allocate the Cobhan output buffer with estimated length plus Cobhan buffer header size
  unsigned char *cobhanOutputBuffer = new unsigned char[bufferSize + header_size];

  //Initialize the Cobhan output buffer header
  *((int32_t*)cobhanOutputBuffer) = bufferSize;
  *((int32_t*)(cobhanOutputBuffer+sizeof(int32_t))) = 0;

  //extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void* dataPtr);
  GoInt32 result = DecryptFromJson(partitionIdCobhanBuffer, inputJsonCobhanBuffer, cobhanOutputBuffer);
  delete[] partitionIdCobhanBuffer;
  //delete[] inputJsonCobhanBuffer;
  if (result < 0) {
      delete[] cobhanOutputBuffer;
      Napi::TypeError::New(env, DecryptFailedMessage + std::to_string(result) + " " + std::string(inputJsonCobhanBuffer + header_size, 0, utf8_length))
        .ThrowAsJavaScriptException();
      return env.Null();
  }

  napi_value output;
  //Using C function because it allows length delimited input
  status = napi_create_string_utf8(env, ((const char*) cobhanOutputBuffer) + header_size, *((int*) cobhanOutputBuffer), &output);
  delete[] cobhanOutputBuffer;

  if(status != napi_ok) {
    Napi::TypeError::New(env, EncryptStringFailedMessage)
      .ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::Value(env, output);
}

//extern void Shutdown();
Napi::Value Napi_Shutdown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Shutdown();
  return env.Null();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "Napi_SetupJson"), Napi::Function::New(env, Napi_SetupJson));
  exports.Set(Napi::String::New(env, "Napi_EncryptFromBufferToJson"), Napi::Function::New(env, Napi_EncryptFromBufferToJson));
  exports.Set(Napi::String::New(env, "Napi_EncryptFromStringToJson"), Napi::Function::New(env, Napi_EncryptFromStringToJson));
  exports.Set(Napi::String::New(env, "Napi_DecryptFromJsonToBuffer"), Napi::Function::New(env, Napi_DecryptFromJsonToBuffer));
  exports.Set(Napi::String::New(env, "Napi_DecryptFromJsonToString"), Napi::Function::New(env, Napi_DecryptFromJsonToString));
  exports.Set(Napi::String::New(env, "Napi_Shutdown"), Napi::Function::New(env, Napi_Shutdown));
  return exports;
}

NODE_API_MODULE(napiasherah, Init)
