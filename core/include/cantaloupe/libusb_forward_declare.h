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
#ifndef LIBUSB_FORWARD_DECLARE_H_
#define LIBUSB_FORWARD_DECLARE_H_

// Lets cheat.  We don't want to drag along the libUSB header everywhere, so we forward declare the bits we need.
extern "C"
{
struct libusb_context;
struct libusb_device;
struct libusb_device_handle;
struct libusb_transfer;

void libusb_close(libusb_device_handle*);
void libusb_exit(libusb_context*);
int libusb_release_interface(libusb_device_handle*, int);
}

#endif  // ifndef LIBUSB_FORWARD_DECLARE_H_
