#pragma once

#include <memory>

#include "loggers/loggers_util.h"

namespace spdlog {
class logger;
}  // namespace spdlog

namespace terrier::optimizer {
extern std::shared_ptr<spdlog::logger> optimizer_logger;  // NOLINT

void InitOptimizerLogger();
}  // namespace terrier::optimizer

#define OPTIMIZER_LOG_TRACE(...) ::terrier::optimizer::optimizer_logger->trace(__VA_ARGS__);

#define OPTIMIZER_LOG_DEBUG(...) ::terrier::optimizer::optimizer_logger->debug(__VA_ARGS__);

#define OPTIMIZER_LOG_INFO(...) ::terrier::optimizer::optimizer_logger->info(__VA_ARGS__);

#define OPTIMIZER_LOG_WARN(...) ::terrier::optimizer::optimizer_logger->warn(__VA_ARGS__);

#define OPTIMIZER_LOG_ERROR(...) ::terrier::optimizer::optimizer_logger->error(__VA_ARGS__);
