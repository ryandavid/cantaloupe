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
#ifndef USB_WRAPPER_H_
#define USB_WRAPPER_H_

#include <cstdint>
#include <memory>

// Lets cheat.  We don't want to drag along the libUSB header everywhere, so we forward declare the bits we need.
extern "C"
{
struct libusb_context;
void libusb_exit(libusb_context* context);
}

namespace cantaloupe
{

// Deleter for the smart pointer that owns the LibUSB context.
struct libUsbContextDeleter
{
  void operator()(libusb_context* context) { libusb_exit(context); }
};

class UsbWrapper
{
 public:
  static constexpr uint16_t kUsbVendorId = 0x1d50;
  static constexpr uint16_t kUsbProductId = 0x606f;

  UsbWrapper();

  void listDevices();

  const char* getLibUsbVersion() const;

 private:
  std::unique_ptr<libusb_context, libUsbContextDeleter> context_;
};

}  // namespace cantaloupe

#endif  // USB_WRAPPER_H_
