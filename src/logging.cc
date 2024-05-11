#include "logging.h"
#include "hints.h"
#include <iostream>

void Logger::set_verbose_flag(int32_t verbose) { verbose_flag = verbose != 0; }

Logger::Logger(std::string system_name) : system_name(system_name) {
  std::cerr << "Created logger for " << system_name << std::endl << std::flush;
}

void Logger::stderr_debug_log(const char *function_name, const char *message) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

void Logger::stderr_debug_log(const char *function_name,
                         const std::string &message) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name << ": " << message
              << std::endl
              << std::flush;
  }
}

void Logger::stderr_debug_log_alloca(
    const char *function_name, const char *variable_name, size_t length) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name
              << ": Calling alloca(" << length << ") (stack) for "
              << variable_name << std::endl
              << std::flush;
  }
}

void Logger::stderr_debug_log_new(const char *function_name,
                             const char *variable_name, size_t length) const {
  if (unlikely(verbose_flag)) {
    std::cerr << system_name << ": [DEBUG] " << function_name
              << ": Calling new[" << length << "] (heap) for " << variable_name
              << std::endl
              << std::flush;
  }
}

void Logger::stderr_error_log(const char *function_name, const char *message) const {
  std::cerr << system_name << ": [ERROR] " << function_name << ": " << message
            << std::endl
            << std::flush;
}

void Logger::stderr_error_log(const char *function_name,
                         const std::string &message) const {
  std::cerr << system_name << ": [ERROR] " << function_name << ": " << message
            << std::endl
            << std::flush;
}
