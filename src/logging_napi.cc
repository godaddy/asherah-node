#include "logging_napi.h"
#include "napi_utils.h"

LoggerNapi::LoggerNapi(Napi::Env &env, const std::string &system_name)
    : StdErrLogger(system_name), log_hook(Napi::FunctionReference()), env(env) {
}

[[maybe_unused]] LoggerNapi::LoggerNapi(Napi::Env &env,
                                        const std::string &system_name,
                                        Napi::Function new_log_hook)
    : StdErrLogger(system_name), env(env) {
  if (unlikely(new_log_hook.IsEmpty())) {
    NapiUtils::ThrowException(env,
                              system_name + ": new_log_hook cannot be nullptr");
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
    NapiUtils::ThrowException(env,
                              system_name + ": new_log_hook cannot be empty");
  }
  auto old_log_hook = std::exchange(log_hook, Napi::Persistent(new_log_hook));
  if (!old_log_hook.IsEmpty()) {
    old_log_hook.Unref();
  }
}

void LoggerNapi::call_log_hook(int level, const std::string &message) const {
  if (unlikely(log_hook.IsEmpty())) {
    NapiUtils::ThrowException(env, system_name + ": log_hook cannot be empty");
  }
  Napi::HandleScope scope(env);
  Napi::Function log_hook_function = log_hook.Value();
  log_hook_function.Call(
      {Napi::Number::New(env, level),
       Napi::String::New(env, system_name + ": " + message)});
}

void LoggerNapi::debug_log(const char *function_name,
                           const char *message) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      StdErrLogger::debug_log(function_name, message);
    } else {
      call_log_hook(posix_log_level_debug,
                    system_name + ": " + function_name + ": " + message);
    }
  }
}

void LoggerNapi::debug_log(const char *function_name,
                           const std::string &message) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      StdErrLogger::debug_log(function_name, message);
    } else {
      call_log_hook(posix_log_level_debug,
                    system_name + ": " + function_name + ": " + message);
    }
  }
}

void LoggerNapi::debug_log_alloca(const char *function_name,
                                  const char *variable_name,
                                  size_t length) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      StdErrLogger::debug_log_alloca(function_name, variable_name, length);
    } else {
      call_log_hook(posix_log_level_debug,
                    system_name + ": " + function_name + ": Calling alloca(" +
                        std::to_string(length) + ") (stack) for " +
                        variable_name);
    }
  }
}

void LoggerNapi::debug_log_new(const char *function_name,
                               const char *variable_name, size_t length) const {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      StdErrLogger::debug_log_new(function_name, variable_name, length);
    } else {
      call_log_hook(posix_log_level_debug, system_name + ": " + function_name +
                                               ": Calling new[" +
                                               std::to_string(length) +
                                               "] (heap) for " + variable_name);
    }
  }
}

void LoggerNapi::error_log(const char *function_name,
                           const char *message) const {
  if (likely(!log_hook.IsEmpty())) {
    call_log_hook(posix_log_level_error,
                  system_name + ": " + function_name + ": " + message);
  }
}

void LoggerNapi::error_log(const char *function_name,
                           const std::string &message) const {
  if (likely(!log_hook.IsEmpty())) {
    call_log_hook(posix_log_level_error,
                  system_name + ": " + function_name + ": " + message);
  }
}
