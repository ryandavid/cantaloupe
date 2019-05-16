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
#ifndef CANABLE_CMDS_H_
#define CANABLE_CMDS_H_

#include <cstdint>

namespace cantaloupe
{

struct __attribute__((packed)) candlelightHostConfig
{
  constexpr candlelightHostConfig() :
    byte_order{0}
  {
  }

  uint32_t byte_order;
};

static_assert(sizeof(candlelightHostConfig) == 4, "candlelightHostConfig is not properly represented.");

struct __attribute__((packed)) candleLightDeviceConfig
{
  constexpr candleLightDeviceConfig() :
    reserved1{0},
    reserved2{0},
    reserved3{0},
    icount{0},
    sw_version{0},
    hw_version{0}
  {
  }

  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t icount;
  uint32_t sw_version;
  uint32_t hw_version;
};

static_assert(sizeof(candleLightDeviceConfig) == 12, "candleLightDeviceConfig is not properly represented.");

struct __attribute__((packed)) candleDeviceMode
{
  constexpr candleDeviceMode() :
    mode{0},
    flags{0}
  {
  }

  uint32_t mode;
  uint32_t flags;
};

static_assert(sizeof(candleDeviceMode) == 8, "candleDeviceMode is not properly represented.");

struct __attribute__((packed)) candleHostFrame
{
  constexpr candleHostFrame() :
    echo_id{0},
    can_id{0},
    can_dlc{0},
    channel{0},
    flags{0},
    reserved{0},
    data{},
    timestamp_us{0}
  {
  }

  uint32_t echo_id;
  uint32_t can_id;

  uint8_t can_dlc;
  uint8_t channel;
  uint8_t flags;
  uint8_t reserved;

  uint8_t data[8];

  uint32_t timestamp_us;
};

static_assert(sizeof(candleHostFrame) == 24, "candleHostFrame is not properly represented.");

struct __attribute__((packed)) candleDeviceBitTiming
{
  constexpr candleDeviceBitTiming() :
    prop_seg{0},
    phase_seg1{0},
    phase_seg2{0},
    sjw{0},
    brp{0}
  {
  }

  uint32_t prop_seg;
  uint32_t phase_seg1;
  uint32_t phase_seg2;
  uint32_t sjw;
  uint32_t brp;
};

enum GsUsbBreq : uint8_t
{
  HOST_FORMAT = 0,
  BITTIMING,
  MODE,
  BERR,
  BT_CONST,
  DEVICE_CONFIG,
  TIMESTAMP,
  IDENTIFY,
  GET_USER_ID,
  SET_USER_ID,
};

constexpr uint32_t kGsCanModeFlagNormal = 0;
constexpr uint32_t kGsCanModeFlagListenOnly = (1UL << 0);
constexpr uint32_t kGsCanModeFlagLoopBack = (1UL << 1);
constexpr uint32_t kGsCanModeFlagTripleSample = (1UL << 2);
constexpr uint32_t kGsCanModeFlagOneShot = (1UL << 3);
constexpr uint32_t kGsCanModeFlagHwTimestamp = (1UL << 4);
constexpr uint32_t kGsCanModeFlagPadPacketsToMaxPacketSize = (1UL << 7);

constexpr uint32_t kGsCanModeReset = 0;
constexpr uint32_t kGSCanModeStart = 1;

constexpr uint32_t kGsCanExpectedHostMagic = 0x0000BEEF;

}  // namespace cantaloupe

#endif  // ifndef CANABLE_CMDS_H_
