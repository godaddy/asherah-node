#ifndef LOGGING_NAPI_H
#define LOGGING_NAPI_H

#include "hints.h"
#include "logging.h"
#include "logging_stderr.h"
#include <napi.h>

class LoggerNapi : public StdErrLogger {
public:
  LoggerNapi(Napi::Env &env, const std::string &system_name);

  [[maybe_unused]] explicit LoggerNapi(Napi::Env &env,
                                       const std::string &system_name,
                                       Napi::Function new_log_hook);
  ~LoggerNapi();

  void set_log_hook(Napi::Function new_log_hook);

  void debug_log(const char *function_name, const char *message) const override;
  void debug_log(const char *function_name,
                 const std::string &message) const override;
  void debug_log_alloca(const char *function_name, const char *variable_name,
                        size_t length) const override;

  void debug_log_new(const char *function_name, const char *variable_name,
                     size_t length) const override;

  void error_log(const char *function_name, const char *message) const override;
  void error_log(const char *function_name,
                 const std::string &message) const override;

private:
  void call_log_hook(int level, const std::string &message) const;
  Napi::FunctionReference log_hook;
  Napi::Env env;
  const int posix_log_level_error = 3;
  const int posix_log_level_debug = 7;
};

#endif // LOGGING_NAPI_H
