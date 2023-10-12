#include "logging.h"

Logger::Logger() {}

Logger::Logger(Napi::Function new_log_hook) {
  if (unlikely(new_log_hook.IsEmpty())) {
    throw new std::runtime_error("new_log_hook cannot be nullptr");
  }
  log_hook = Napi::Persistent(new_log_hook);
}

Logger::~Logger() {
  if (!log_hook.IsEmpty()) {
    log_hook.Unref();
  }
}

void Logger::set_log_hook(Napi::Function new_log_hook) {
  if (unlikely(new_log_hook.IsEmpty())) {
    throw new std::runtime_error("new_log_hook cannot be nullptr");
  }
  if (!log_hook.IsEmpty()) {
    log_hook.Unref();
  }
  log_hook = Napi::Persistent(new_log_hook);
}

void Logger::set_verbose_flag(int32_t verbose) { verbose_flag = verbose; }

void Logger::debug_log(const char *function_name, const char *message) {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log(function_name, message);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env, asherah_node_prefix + function_name + ": " +
                                      message)});
    }
  }
}

void Logger::debug_log(const char *function_name, std::string message) {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log(function_name, message);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env, asherah_node_prefix + function_name + ": " +
                                      message)});
    }
  }
}

void Logger::debug_log_alloca(const char *function_name,
                              const char *variable_name, size_t length) {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log_alloca(function_name, variable_name, length);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env, asherah_node_prefix + function_name +
                                      ": Calling alloca(" +
                                      std::to_string(length) +
                                      ") (stack) for " + variable_name)});
    }
  }
}

void Logger::debug_log_new(const char *function_name, const char *variable_name,
                           size_t length) {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_debug_log_new(function_name, variable_name, length);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_debug),
           Napi::String::New(env, asherah_node_prefix + function_name +
                                      ": Calling new[" +
                                      std::to_string(length) + "] (heap) for " +
                                      variable_name)});
    }
  }
}

void Logger::debug_log_copy_buffer(const char *function_name,
                                   const char *buffer, size_t length) {
  if (unlikely(verbose_flag)) {
    debug_log(__func__, "Copying " + std::to_string(length) + " bytes to " +
                            format_ptr(buffer) + " - " +
                            format_ptr(buffer + length));
  }
}

void Logger::debug_log_configure_cbuffer(const char *function_name,
                                         const char *variable_name,
                                         size_t length) {
  if (unlikely(verbose_flag)) {
    debug_log(__func__, "configure_cbuffer(" + format_ptr(variable_name) +
                            ", " + std::to_string(length) + ")");
  }
}

void Logger::error_log(const char *function_name, const char *message) {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_error_log(function_name, message);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_error),
           Napi::String::New(env, asherah_node_prefix + function_name + ": " +
                                      message)});
    }
  }
}

void Logger::error_log(const char *function_name, std::string message) {
  if (unlikely(verbose_flag)) {
    if (unlikely(log_hook.IsEmpty())) {
      stderr_error_log(function_name, message);
    } else {
      Napi::Env env = log_hook.Env();
      Napi::HandleScope scope(env);
      Napi::Function log_hook_function = log_hook.Value();
      log_hook_function.Call(
          {Napi::Number::New(env, posix_log_level_error),
           Napi::String::New(env, asherah_node_prefix + function_name + ": " +
                                      message)});
    }
  }
}

__attribute__((always_inline)) inline void
Logger::stderr_debug_log(const char *function_name, const char *message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void
Logger::stderr_debug_log(const char *function_name, std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void
Logger::stderr_debug_log_alloca(const char *function_name,
                                const char *variable_name, size_t length) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name
              << ": Calling alloca(" << length << ") (stack) for "
              << variable_name << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void
Logger::stderr_debug_log_new(const char *function_name,
                             const char *variable_name, size_t length) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [DEBUG] " << function_name << ": Calling new["
              << length << "] (heap) for " << variable_name << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void
Logger::stderr_error_log(const char *function_name, const char *message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [ERROR] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline void
Logger::stderr_error_log(const char *function_name, std::string message) {
  if (unlikely(verbose_flag)) {
    std::cerr << "asherah-node: [ERROR] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

__attribute__((always_inline)) inline std::string
Logger::format_ptr(const char *ptr) {
  std::ostringstream ss;
  ss << "0x" << std::hex << (intptr_t)ptr;
  return ss.str();
}

__attribute__((always_inline, noreturn)) inline void
Logger::log_error_and_throw(const char *function_name, std::string error_msg) {
  error_log(function_name, error_msg);
  throw new std::runtime_error(function_name + (": " + error_msg));
}
