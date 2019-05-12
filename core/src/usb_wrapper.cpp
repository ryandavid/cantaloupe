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

#include <cantaloupe/usb_wrapper.h>
#include <cantaloupe/log.h>

#include <stdexcept>

#include <libusb.h>

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

namespace cantaloupe
{

UsbWrapper::UsbWrapper() :
  context_{nullptr}
{
  libusb_context* temp_context;
  if (libusb_init(&temp_context) != 0)
  {
    throw std::runtime_error("Failed to create LibUSB context.");
  }

  // Stuff it into smart pointer.
  context_.reset(temp_context);
}

const char* UsbWrapper::getLibUsbVersion() const
{
  return STRINGIFY(LIBUSB_API_VERSION);
}

void UsbWrapper::listDevices()
{
  libusb_device **list;

  ssize_t cnt = libusb_get_device_list(context_.get(), &list);

  if (cnt < 0)
  {
    CANTALOUPE_ERROR("No USB devices found.");
    return;
  }

  CANTALOUPE_INFO("Found {} devices.", cnt);

  for (ssize_t i = 0; i < cnt; i++)
  {
    libusb_device *device = list[i];

    libusb_device_descriptor desc;

    if (libusb_get_device_descriptor(device, &desc) == 0)
    {
      CANTALOUPE_INFO("Device {} : VID = 0x{:04X}, PID = 0x{:04X}", i, desc.idVendor, desc.idProduct);
    }
  }

  libusb_free_device_list(list, 1);
}

}  // namespace cantaloupe
