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

#include <can-macos/log.h>
#include <can-macos/usb_wrapper.h>

int main()
{
  can_macos::UsbWrapper usb;
  CORE_INFO("LibUSB version '{}'.", usb.getLibUsbVersion());

  usb.listDevices();

  return 0;
}
