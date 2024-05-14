//
// Created by Jeremiah Gowdy on 5/14/24.
//

#include "logging_stderr.h"
#include "hints.h"
#include <iostream>

StdErrLogger::StdErrLogger(const std::string &system_name)
    : Logger(system_name) {}

void StdErrLogger::debug_log(const char *function_name,
                             const char *message) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

void StdErrLogger::debug_log(const char *function_name,
                             const std::string &message) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

void StdErrLogger::debug_log_alloca(const char *function_name,
                                    const char *variable_name,
                                    size_t length) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name
              << ": Calling alloca(" << length << ") (stack) for "
              << variable_name << std::endl
              << std::flush;
  }
}

void StdErrLogger::debug_log_new(const char *function_name,
                                 const char *variable_name,
                                 size_t length) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name
              << ": Calling new[" << length << "] (heap) for " << variable_name
              << std::endl
              << std::flush;
  }
}

void StdErrLogger::error_log(const char *function_name,
                             const char *message) const {
  std::cerr << system_name << ": [ERROR] " << function_name << ": " << message
            << std::endl
            << std::flush;
}

void StdErrLogger::error_log(const char *function_name,
                             const std::string &message) const {
  std::cerr << system_name << ": [ERROR] " << function_name << ": " << message
            << std::endl
            << std::flush;
}
