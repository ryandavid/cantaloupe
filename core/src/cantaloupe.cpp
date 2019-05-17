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

#include <cantaloupe/log.h>
#include <cantaloupe/usb_wrapper.h>

#include <array>

int main()
{
  cantaloupe::UsbWrapper usb;
  if (usb.isConnected() == false)
  {
    CANTALOUPE_ERROR("No devices found.");
    return -1;
  }

  if (usb.setBitrate(500000) == false)
  {
    CANTALOUPE_ERROR("Failed to set channel bitrate.");
    return -1;
  }

  if (usb.startChannel(true, true) == false)
  {
    CANTALOUPE_ERROR("Failed to start the CAN channel.");
    return -1;
  }

  // Create a dummy CAN frame.
  cantaloupe::CanFrame tx_frame;
  tx_frame.id = 0x123;
  tx_frame.dlc = 8;

  for (size_t i = 0; i < tx_frame.data.size(); ++i)
  {
    tx_frame.data[i] = static_cast<uint8_t>(i);
  }

  if (usb.writeCanFrame(tx_frame) == false)
  {
    CANTALOUPE_ERROR("Failed to write to the CAN channel.");
    return -1;
  }

  cantaloupe::CanFrame rx_frame;
  if (usb.readCanFrame(&rx_frame) == false)
  {
    CANTALOUPE_ERROR("Failed to read from the CAN channel.");
    return -1;
  }

  // Print the received CAN frame out.
  CANTALOUPE_INFO("Received frame ID 0x{:02X}, dlc = {}, data = [{}, {}, {}, {}, {}, {}, {}, {}], ts = {} us.",
    rx_frame.id, rx_frame.dlc, rx_frame.data[0], rx_frame.data[1], rx_frame.data[2], rx_frame.data[3], rx_frame.data[4],
    rx_frame.data[5], rx_frame.data[6], rx_frame.data[7], rx_frame.timestamp_us);

  if (usb.startChannel(false) == false)
  {
    CANTALOUPE_ERROR("Failed to stop the CAN channel.");
    return -1;
  }

  CANTALOUPE_INFO("All done!");
  return 0;
}
