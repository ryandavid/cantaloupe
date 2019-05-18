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
#ifndef CAN_FRAME_H_
#define CAN_FRAME_H_

#include <array>
#include <cstdint>

namespace cantaloupe
{
// Make our own representation of the CAN frame rather relying on the USB interface protocol definition.  It looks
// essentially the same though.
struct CanFrame
{
  constexpr CanFrame() :
    id{0},
    dlc{0},
    data{},
    timestamp_us{0}
  {
  }

  // Maximum number of bytes able to be represented in a CAN frame.
  static constexpr size_t kDataNumMaxBytes = 8;

  // Message ID.
  uint32_t id;

  // Data length.
  uint8_t dlc;

  // Data in message.
  std::array<uint8_t, kDataNumMaxBytes> data;

  // Device timestamp on message receipt.
  uint32_t timestamp_us;
};

}  // namespace cantaloupe

#endif  // ifndef CAN_FRAME_H_
