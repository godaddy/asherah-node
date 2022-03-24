#include <napi.h>
#include "../lib/libasherah.h"

//extern GoInt32 SetupJson(void* configJson);
Napi::Value Napi_SetupJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer configJson = info[0].As<Napi::Buffer<unsigned char>>();

  GoInt32 result = SetupJson(configJson.Data());

  Napi::Number num = Napi::Number::New(env, result);
  return num;
}

//extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr, void* jsonPtr);
Napi::Value Napi_EncryptToJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsBuffer() || !info[1].IsBuffer() || !info[2].IsBuffer()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<unsigned char> partitionId = info[0].As<Napi::Buffer<unsigned char>>();
  Napi::Buffer<unsigned char> data = info[1].As<Napi::Buffer<unsigned char>>();
  Napi::Buffer<unsigned char> outputJson = info[2].As<Napi::Buffer<unsigned char>>();

  GoInt32 result = EncryptToJson(partitionId.Data(), data.Data(), outputJson.Data());

  Napi::Number num = Napi::Number::New(env, result);
  return num;
}

//extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr, void* dataPtr);
Napi::Value Napi_DecryptFromJson(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsBuffer() || !info[1].IsBuffer() || !info[2].IsBuffer()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Buffer<unsigned char> partitionId = info[0].As<Napi::Buffer<unsigned char>>();
  Napi::Buffer<unsigned char> inputJson = info[1].As<Napi::Buffer<unsigned char>>();
  Napi::Buffer<unsigned char> outputData = info[2].As<Napi::Buffer<unsigned char>>();

  GoInt32 result = DecryptFromJson(partitionId.Data(), inputJson.Data(), outputData.Data());

  Napi::Number num = Napi::Number::New(env, result);
  return num;
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
