#ifndef NAPI_UTILS_H
#define NAPI_UTILS_H

#include "hints.h"
#include <napi.h>
#include <stdexcept>

#ifndef NAPI_CPP_EXCEPTIONS
#error Support for C++ exceptions is required
#endif

class NapiUtils {
public:
  // This gets the length of the utf-8 string without allocating / copying
  static size_t GetUtf8StringLength(const Napi::Env &env,
                             const Napi::String &napiString) {
    size_t result;
    napi_status status =
        napi_get_value_string_utf8(env, napiString, nullptr, 0, &result);
    if (status != napi_ok) {
      ThrowException(env, "Failed to get UTF-8 string length. Status: " +
                              std::to_string(status));
    }
    return result;
  }

  [[noreturn]] static void ThrowException(const Napi::Env &env,
                                          const std::string &message) {
    auto error = Napi::Error::New(env, message);
    error.ThrowAsJavaScriptException();
    // If the JavaScript exception isn't thrown, fallback to a C++ exception
    throw error;
  }

  static void AsJsonObjectAndString(const Napi::Env &env,
                                    const Napi::Value &value,
                                    Napi::String &jsonString,
                                    Napi::Object &jsonObject) {
    auto json = env.Global().Get("JSON").As<Napi::Object>();
    auto jsonStringify = json.Get("stringify").As<Napi::Function>();
    auto jsonParse = json.Get("parse").As<Napi::Function>();
    if (unlikely(value.IsUndefined())) {
      ThrowException(env, "Input value is undefined");
    } else if (value.IsString()) {
      // Convert string to object using JSON.parse
      Napi::String str = value.As<Napi::String>();
      jsonString = str;
      jsonObject = jsonParse.Call({str}).As<Napi::Object>();
    } else if (value.IsObject()) {
      // Convert object to string using JSON.stringify
      Napi::Object obj = value.As<Napi::Object>();
      if (jsonStringify.IsUndefined()) {
        ThrowException(env, "jsonStringify is undefined");
      } else if (jsonStringify.IsEmpty()) {
        ThrowException(env, "jsonStringify is empty");
      }
      jsonString = jsonStringify.Call({obj}).As<Napi::String>();
      jsonObject = obj;
    } else {
      ThrowException(env, "Input value must be a Napi::Object or Napi::String");
    }
  }

#pragma region Object Properties

  static void GetStringProperty(const Napi::Object &obj,
                                const char *propertyName,
                                Napi::String &result) {
    auto maybeValue = obj.Get(propertyName);

    if (likely(!maybeValue.IsUndefined() && !maybeValue.IsNull() &&
               maybeValue.IsString())) {
      result = maybeValue.As<Napi::String>();
    } else {
      ThrowException(obj.Env(), "Property '" + std::string(propertyName) +
                                    "' is not a Napi::String or is missing.");
    }
  }

  static void GetBooleanProperty(const Napi::Object &obj,
                                 const char *propertyName, bool &result,
                                 bool defaultValue = false) {
    auto maybeValue = obj.Get(propertyName);

    if (likely(!maybeValue.IsEmpty())) {
      Napi::Value value = maybeValue;

      if (value.IsBoolean()) {
        result = value.As<Napi::Boolean>();
      } else {
        // Coerce to boolean
        result = value.ToBoolean();
      }
    } else {
      result = defaultValue;
    }
  }

#pragma endregion Object Properties

#pragma region Parameter Support

  static void RequireParameterCount(const Napi::CallbackInfo &info,
                                    size_t expected) {
    if (unlikely(info.Length() != expected)) {
      std::string error_msg = "Expected " + std::to_string(expected) +
                              " arguments, but got " +
                              std::to_string(info.Length());
      ThrowException(info.Env(), error_msg);
    }
  }

  static Napi::String RequireParameterString(const Napi::Env &env,
                                             const char *func_name,
                                             Napi::Value value) {
    if (likely(value.IsString())) {
      return value.As<Napi::String>();
    } else if (unlikely(value.IsUndefined())) {
      ThrowException(env, "Expected String but received undefined");
    } else if (unlikely(value.IsNull())) {
      ThrowException(env, "Expected String but received null");
    } else {
      ThrowException(env, "Expected String but received unknown type");
    }
  }

  static Napi::Buffer<unsigned char>
  RequireParameterBuffer(const Napi::Env &env, const char *func_name,
                         Napi::Value value) {
    if (likely(value.IsBuffer())) {
      return value.As<Napi::Buffer<unsigned char>>();
    } else if (unlikely(value.IsUndefined())) {
      ThrowException(env, "Expected String but received undefined");
    } else if (unlikely(value.IsNull())) {
      ThrowException(env, "Expected String but received null");
    } else {
      ThrowException(env, "Expected String but received unknown type");
    }
  }

  static Napi::Value RequireParameterStringOrBuffer(const Napi::Env &env,
                                                    const char *func_name,
                                                    Napi::Value value) {
    if (value.IsString()) {
      return value.As<Napi::String>();
    } else if (likely(value.IsBuffer())) {
      return value.As<Napi::Buffer<unsigned char>>();
    } else if (unlikely(value.IsUndefined())) {
      ThrowException(env, "Expected String but received undefined");
    } else if (unlikely(value.IsNull())) {
      ThrowException(env, "Expected String but received null");
    } else {
      ThrowException(env, "Expected String but received unknown type");
    }
  }

#pragma endregion Parameter Support
};

#endif // NAPI_UTILS_H
