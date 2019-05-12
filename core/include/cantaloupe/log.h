// cantaloupe, CAN bus viewer for MacOS.
//
// cantaloupe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// cantaloupe is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with cantaloupe.  If not, see
// <https://www.gnu.org/licenses/>.
#ifndef CAN_MACOS_LOG_H_
#define CAN_MACOS_LOG_H_

#include <spdlog/spdlog.h>

#include <memory>

namespace cantaloupe
{

extern std::shared_ptr<spdlog::logger> g_console_logger;

}  // namespace cantaloupe

#define CANTALOUPE_LOG_IMPL(loglevel, ...) \
  { \
    if (cantaloupe::g_console_logger != nullptr) \
    { \
      cantaloupe::g_console_logger->log(spdlog::source_loc{SPDLOG_FILE_BASENAME(__FILE__), __LINE__}, loglevel, \
        __VA_ARGS__); \
    } \
  }

#define CANTALOUPE_DEBUG(...) CANTALOUPE_LOG_IMPL(spdlog::level::debug, __VA_ARGS__)
#define CANTALOUPE_INFO(...) CANTALOUPE_LOG_IMPL(spdlog::level::info, __VA_ARGS__)
#define CANTALOUPE_WARN(...) CANTALOUPE_LOG_IMPL(spdlog::level::warn, __VA_ARGS__)
#define CANTALOUPE_ERROR(...) CANTALOUPE_LOG_IMPL(spdlog::level::err, __VA_ARGS__)
#define CANTALOUPE_CRITICAL(...) CANTALOUPE_LOG_IMPL(spdlog::level::critical, __VA_ARGS__)

#endif  // CAN_MACOS_LOG_H_
