// can-macos, CAN bus viewer for MacOS.
//
// can-macos is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// can-macos is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with can-macos.  If not, see
// <https://www.gnu.org/licenses/>.
#ifndef CAN_MACOS_LOG_H_
#define CAN_MACOS_LOG_H_

#include <spdlog/spdlog.h>

#include <memory>

namespace can_macos
{

extern std::shared_ptr<spdlog::logger> g_console_logger;

}  // namespace can_macos

#define CORE_LOG_IMPL(loglevel, ...) \
  { \
    if (can_macos::g_console_logger != nullptr) \
    { \
      can_macos::g_console_logger->log(spdlog::source_loc{SPDLOG_FILE_BASENAME(__FILE__), __LINE__}, loglevel, \
        __VA_ARGS__); \
    } \
  }

#define CORE_DEBUG(...) CORE_LOG_IMPL(spdlog::level::debug, __VA_ARGS__)
#define CORE_INFO(...) CORE_LOG_IMPL(spdlog::level::info, __VA_ARGS__)
#define CORE_WARN(...) CORE_LOG_IMPL(spdlog::level::warn, __VA_ARGS__)
#define CORE_ERROR(...) CORE_LOG_IMPL(spdlog::level::err, __VA_ARGS__)
#define CORE_CRITICAL(...) CORE_LOG_IMPL(spdlog::level::critical, __VA_ARGS__)

#endif  // CAN_MACOS_LOG_H_
