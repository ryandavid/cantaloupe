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

#include <can-macos/usb_wrapper.h>

#include <cstdio>

#include <libusb.h>

int main()
{
  const struct libusb_version* version = libusb_get_version();
  printf("LibUSB version %u.%u.%u\n", version->major, version->minor, version->micro);

  libusb_context *context = NULL;
  libusb_init(&context);

  libusb_device **list;
  ssize_t cnt = libusb_get_device_list(context, &list);

  if (cnt < 0)
  {
      printf("Error\n");
      return -1;
  }

  printf("Found %zi devices.\n", cnt);

  for (size_t i = 0; i < cnt; i++)
  {
    libusb_device *device = list[i];

    libusb_device_descriptor desc = {0};

    if (libusb_get_device_descriptor(device, &desc) == 0)
    {
      printf("Device %zu : VID = 0x%04X, PID = 0x%04X\n", i, desc.idVendor, desc.idProduct);
    }
  }

  libusb_free_device_list(list, 1);
  libusb_exit(context);

  return 0;
}
