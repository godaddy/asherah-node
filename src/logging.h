#ifndef LOGGING_H
#define LOGGING_H
#include "hints.h"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <napi.h>
#include <sstream>
#include <string>

class Logger {
public:
  Logger();
  Logger(Napi::Function new_log_hook);
  ~Logger();

  void set_log_hook(Napi::Function new_log_hook);
  void set_verbose_flag(int32_t verbose_flag);

  void debug_log(const char *function_name, const char *message);
  void debug_log(const char *function_name, std::string message);
  void debug_log_alloca(const char *function_name, const char *variable_name,
                        size_t length);

  void debug_log_new(const char *function_name, const char *variable_name,
                     size_t length);
  void debug_log_copy_buffer(const char *function_name,
                             const char *variable_name, size_t length);
  void debug_log_configure_cbuffer(const char *function_name,
                                   const char *variable_name, size_t length);
  void error_log(const char *function_name, const char *message);
  void error_log(const char *function_name, std::string message);
  void log_error_and_throw(const char *function_name, std::string error_msg);

private:
  void stderr_debug_log(const char *function_name, const char *message);
  void stderr_debug_log(const char *function_name, std::string message);
  void stderr_debug_log_alloca(const char *function_name,
                               const char *variable_name, size_t length);
  void stderr_debug_log_new(const char *function_name,
                            const char *variable_name, size_t length);
  void stderr_error_log(const char *function_name, const char *message);
  void stderr_error_log(const char *function_name, std::string message);

  std::string format_ptr(const char *ptr);

  int32_t verbose_flag = 0;
  Napi::FunctionReference log_hook;
  std::string asherah_node_prefix = "asherah-node: ";
  const int posix_log_level_error = 3;
  const int posix_log_level_debug = 7;
};

#endif
