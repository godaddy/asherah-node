#include "logging.h"

[[maybe_unused]] void Logger::set_verbose_flag(int32_t verbose) { verbose_flag = verbose != 0; }

Logger::Logger(std::string system_name) : system_name(std::move(system_name)) {}
