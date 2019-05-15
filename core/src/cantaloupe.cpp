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

enum gs_usb_breq : uint8_t
{
  GS_USB_BREQ_HOST_FORMAT = 0,
  GS_USB_BREQ_BITTIMING,
  GS_USB_BREQ_MODE,
  GS_USB_BREQ_BERR,
  GS_USB_BREQ_BT_CONST,
  GS_USB_BREQ_DEVICE_CONFIG,
  GS_USB_BREQ_TIMESTAMP,
  GS_USB_BREQ_IDENTIFY,
  GS_USB_BREQ_GET_USER_ID,
  GS_USB_BREQ_SET_USER_ID,
};

int main()
{
  CANTALOUPE_INFO("Waiting for hotplug.");
  cantaloupe::UsbWrapper usb;

  std::array<uint8_t, 4> tx_buffer = {0, 0, 0, 1};

  bool result = usb.transmitControl(0, gs_usb_breq::GS_USB_BREQ_IDENTIFY, 0, 0, &tx_buffer[0], tx_buffer.size());
  CANTALOUPE_INFO("Control transfer resulted in {}.", result);

  sleep(1);

  tx_buffer[3] = 0;
  result = usb.transmitControl(0, gs_usb_breq::GS_USB_BREQ_IDENTIFY, 0, 0, &tx_buffer[0], tx_buffer.size());
  CANTALOUPE_INFO("Control transfer resulted in {}.", result);

  return 0;
}
