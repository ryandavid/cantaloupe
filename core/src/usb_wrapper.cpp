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

#include <functional>
#include <stdexcept>

#include <libusb.h>

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

namespace cantaloupe
{

UsbWrapper::UsbWrapper() :
  context_{nullptr},
  hotplug_handle_{0},
  hotplug_thread_shutdown_{false},
  hotplug_thread_{},
  device_mutex_{},
  device_{nullptr}
{
  // Create the necessary LibUSB context.
  libusb_context* temp_context;
  if (libusb_init(&temp_context) != 0)
  {
    throw std::runtime_error("Failed to create LibUSB context.");
  }

  // Stuff it into smart pointer.
  context_.reset(temp_context);

  // Create a lambda to be used when a hotplug event occurs.  This is to prevent the LibUSB API from "poisoning" our
  // public header, yet we stil have access to private methods.
  auto hotplug_callback = [](libusb_context*, libusb_device* dev, libusb_hotplug_event event, void* this_ptr) -> int {
    // Make sure we have a valid `this` pointer.
    if (this_ptr == nullptr)
    {
      return 0;
    }

    // Convert from a LibUSB event type to our own definition.
    UsbEventHotplugEvent converted_event =
      event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED ? UsbEventHotplugEvent::ATTACHED : UsbEventHotplugEvent::DETACHED;

    // Hand off to the real event handler.
    static_cast<UsbWrapper*>(this_ptr)->hotplugEvent(converted_event, dev);
    return 0;
  };

  // Register a hotplug handler with LibUSB.  We are using the lambda above for the callback, and passing along the
  // `this` pointer so we can call into ourselves.
  libusb_hotplug_register_callback(
    context_.get(),  // ctx.
    static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),  // evt.
    LIBUSB_HOTPLUG_NO_FLAGS,  // flags.
    kUsbVendorId,  // device vendor ID.
    kUsbProductId,  // device product ID.
    LIBUSB_HOTPLUG_MATCH_ANY,  // device class to match.
    hotplug_callback,  // callback.
    this,  // user data.
    &hotplug_handle_  // handle.
  );

  // Create a thread that monitors for hotplug events.
  hotplug_thread_ = std::thread(std::bind(&UsbWrapper::hotplugMonitorThread, this));
}

UsbWrapper::~UsbWrapper()
{
  // If the hotplug monitor thread is still running, signal it to die and then wait for it.
  if (hotplug_thread_.joinable() == true)
  {
    hotplug_thread_shutdown_ = true;
    hotplug_thread_.join();
  }

  // Deregister the hotplug callback.
  libusb_hotplug_deregister_callback(context_.get(), hotplug_handle_);
}

const char* UsbWrapper::getLibUsbVersion() const
{
  return STRINGIFY(LIBUSB_API_VERSION);
}

void UsbWrapper::hotplugMonitorThread() const
{
  while (hotplug_thread_shutdown_ == false)
  {
    // Use a timeout so that way there is a opportunity for this thread to be signaled to be closed.
    timeval duration;
    duration.tv_sec = kHotplugEventTimeoutSecs;
    duration.tv_usec = 0;

    libusb_handle_events_timeout(context_.get(), &duration);
  }
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

void UsbWrapper::hotplugEvent(UsbEventHotplugEvent event, libusb_device* dev)
{
  libusb_device_descriptor desc;
  libusb_get_device_descriptor(dev, &desc);

  if (event == UsbEventHotplugEvent::ATTACHED)
  {
    CANTALOUPE_INFO("VID = 0x{:04X} PID = 0x{:04X} arrived.", desc.idVendor, desc.idProduct);

    std::lock_guard<std::mutex> lock(device_mutex_);
    device_ = dev;
  }
  else if (event == UsbEventHotplugEvent::DETACHED)
  {
    CANTALOUPE_INFO("VID = 0x{:04X} PID = 0x{:04X} left.", desc.idVendor, desc.idProduct);

    std::lock_guard<std::mutex> lock(device_mutex_);
    device_ = nullptr;
  }
}

}  // namespace cantaloupe
