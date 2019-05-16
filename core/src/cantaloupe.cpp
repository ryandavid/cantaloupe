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

  if (usb.setIdentifyLeds(true) == false)
  {
    CANTALOUPE_ERROR("Failed to set identify LEDs.");
    return -1;
  }

  sleep(1);

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

  sleep(1);

  if (usb.startChannel(false) == false)
  {
    CANTALOUPE_ERROR("Failed to stop the CAN channel.");
    return -1;
  }

  CANTALOUPE_INFO("All done!");
  return 0;
}
