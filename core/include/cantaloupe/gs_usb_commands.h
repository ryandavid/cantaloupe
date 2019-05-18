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
#ifndef GS_USB_COMMANDS_H_
#define GS_USB_COMMANDS_H_

#include <cstdint>

namespace cantaloupe
{

// Used to let the device determine the host endianess.
struct __attribute__((packed)) GsHostConfig
{
  static constexpr uint32_t kGsCanExpectedHostMagic = 0x0000BEEF;

  constexpr GsHostConfig() :
    byte_order{kGsCanExpectedHostMagic}
  {
  }

  uint32_t byte_order;
};

static_assert(sizeof(GsHostConfig) == 4, "GsHostConfig is not properly represented.");

// Set the device mode (ie Loopback or enable timestamping).
struct __attribute__((packed)) GsDeviceMode
{
  static constexpr uint32_t kFlagNormal = 0;
  static constexpr uint32_t kFlagListenOnly = (1UL << 0);
  static constexpr uint32_t kFlagLoopBack = (1UL << 1);
  static constexpr uint32_t kFlagTripleSample = (1UL << 2);
  static constexpr uint32_t kFlagOneShot = (1UL << 3);
  static constexpr uint32_t kFlagHwTimestamp = (1UL << 4);
  static constexpr uint32_t kFlagPadPacketsToMaxPacketSize = (1UL << 7);

  static constexpr uint32_t kModeReset = 0;
  static constexpr uint32_t kModeStart = 1;

  constexpr GsDeviceMode() :
    mode{0},
    flags{0}
  {
  }

  uint32_t mode;
  uint32_t flags;
};

static_assert(sizeof(GsDeviceMode) == 8, "GsDeviceMode is not properly represented.");

// Frame representation we expect from the device.
struct __attribute__((packed)) GsHostCanFrame
{
  constexpr GsHostCanFrame() :
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

static_assert(sizeof(GsHostCanFrame) == 24, "GsHostCanFrame is not properly represented.");

// Representation of the CAN bus bittiming on the device.
struct __attribute__((packed)) GsDeviceBitTiming
{
  constexpr GsDeviceBitTiming() :
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

// Commands lifted from the Candelight firmware.
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

}  // namespace cantaloupe

#endif  // ifndef GS_USB_COMMANDS_H_
