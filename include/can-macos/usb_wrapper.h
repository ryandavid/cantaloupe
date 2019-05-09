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
#ifndef USB_WRAPPER_H_
#define USB_WRAPPER_H_

#include <cstdint>

#include <libusb.h>

namespace can_macos
{
constexpr uint16_t kUsbVendorId = 0x1d50;
constexpr uint16_t kUsbProductId = 0x606f;

class UsbWrapper
{
 public:
  //constexpr UsbWrapper()

 private:
};

}  // namespace can_macos

#endif  // USB_WRAPPER_H_