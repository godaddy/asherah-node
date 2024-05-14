//
// Created by Jeremiah Gowdy on 5/14/24.
//

#ifndef STDERR_LOGGER_H
#define STDERR_LOGGER_H

#include "logging.h"

class StdErrLogger : public Logger {
public:
  StdErrLogger() = delete;
  explicit StdErrLogger(const std::string &system_name);

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
};

#endif // STDERR_LOGGER_H
