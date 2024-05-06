#include "logging_napi.h"
#include "napi_utils.h"

LoggerNapi::LoggerNapi(Napi::Env &env, std::string system_name)
    : Logger(system_name), log_hook(Napi::FunctionReference()), env(env) {}

LoggerNapi::LoggerNapi(Napi::Env &env, std::string system_name,
               Napi::Function new_log_hook)
    : Logger(system_name), env(env) {
  if (unlikely(new_log_hook.IsEmpty())) {
    NapiUtils::ThrowException(env, "new_log_hook cannot be nullptr");
  }
  log_hook = Napi::Persistent(new_log_hook);
}

LoggerNapi::~LoggerNapi() {
  auto old_log_hook = std::exchange(log_hook, Napi::FunctionReference());
  if (!old_log_hook.IsEmpty()) {
    old_log_hook.Unref();
  }
}

void LoggerNapi::set_log_hook(Napi::Function new_log_hook) {
  if (unlikely(new_log_hook.IsUndefined() || new_log_hook.IsEmpty())) {
    NapiUtils::ThrowException(env, "new_log_hook cannot be empty");
  }
  auto old_log_hook = std::exchange(log_hook, Napi::Persistent(new_log_hook));
  if (!old_log_hook.IsEmpty()) {
    old_log_hook.Unref();
  }
}

void LoggerNapi::debug_log(const char *function_name, const char *message) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log(function_name, message);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env,
                             system_name + function_name + ": " + message)});
    }
  }
}

void LoggerNapi::debug_log(const char *function_name,
                       const std::string &message) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log(function_name, message);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env,
                             system_name + function_name + ": " + message)});
    }
  }
}

void LoggerNapi::debug_log_alloca(const char *function_name,
                              const char *variable_name, size_t length) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log_alloca(function_name, variable_name, length);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env, system_name + function_name +
                                      ": Calling alloca(" +
                                      std::to_string(length) +
                                      ") (stack) for " + variable_name)});
    }
  }
}

void LoggerNapi::debug_log_new(const char *function_name, const char *variable_name,
                           size_t length) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log_new(function_name, variable_name, length);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env, system_name + function_name +
                                      ": Calling new[" +
                                      std::to_string(length) + "] (heap) for " +
                                      variable_name)});
    }
  }
}

void LoggerNapi::error_log(const char *function_name, const char *message) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_error_log(function_name, message);
    } else {
      stderr_error_log(function_name, message);
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_error),
           Napi::String::New(env,
                             system_name + function_name + ": " + message)});
    }
  }
}

void LoggerNapi::error_log(const char *function_name,
                       const std::string &message) const {
  // Unconditionally log errors to stderr
  stderr_error_log(function_name, message);

  if (likely(!log_hook.IsEmpty())) {
    Napi::Env env = log_hook.Env();
    Napi::HandleScope scope(env);
    Napi::Function log_hook_function = log_hook.Value();
    log_hook_function.Call(
        {Napi::Number::New(env, posix_log_level_error),
         Napi::String::New(env, system_name + function_name + ": " + message)});
  }
}

__attribute__((always_inline, noreturn)) inline void
LoggerNapi::log_error_and_throw(const char *function_name,
                            const std::string &error_msg) const {
  std::string final_error_msg =
      system_name + ": [EXCEPTION] " + function_name + (": " + error_msg);

  // Unconditionally log errors to stderr
  stderr_error_log(function_name, final_error_msg);

  error_log(function_name, final_error_msg);

  NapiUtils::ThrowException(env, final_error_msg);
}
