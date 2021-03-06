#pragma once

#ifdef ENABLE_ASYNCIO_LOG
#include "common.hpp"
#include <spdlog/spdlog.h>
BEGIN_ASYNCIO_NAMESPACE
static auto _async_io_logger_ = spdlog::get("asyncio")
                                    ? spdlog::get("asyncio")
                                    : spdlog::stderr_logger_mt("asyncio");
END_ASYNCIO_NAMESPACE
#define LOG_DEBUG(fmt, ...)                                                    \
  ::ASYNCIO_NAMESPACE::_async_io_logger_->debug(                               \
      fmt " - {}:{}:", ##__VA_ARGS__, __FILE__, __LINE__)
#define LOG_INFO(fmt, ...)                                                     \
  ::ASYNCIO_NAMESPACE::_async_io_logger_->info(fmt " - {}:{}:", ##__VA_ARGS__, \
                                               __FILE__, __LINE__)
#define LOG_WARN(fmt, ...)                                                     \
  ::ASYNCIO_NAMESPACE::_async_io_logger_->warn(fmt " - {}:{}:", ##__VA_ARGS__, \
                                               __FILE__, __LINE__)
#define LOG_ERROR(fmt, ...)                                                    \
  ::ASYNCIO_NAMESPACE::_async_io_logger_->error(                               \
      fmt " - {}:{}:", ##__VA_ARGS__, __FILE__, __LINE__)
#else
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#endif