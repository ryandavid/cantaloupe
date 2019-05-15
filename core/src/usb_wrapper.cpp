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

namespace cantaloupe
{

UsbWrapper::UsbWrapper() :
  context_{nullptr},
  hotplug_handle_{0},
  hotplug_thread_shutdown_{false},
  hotplug_thread_{},
  device_handle_mutex_{},
  device_handle_{nullptr}
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
    if ((this_ptr == nullptr) || (dev == nullptr))
    {
      return 0;
    }

    // Hand off to the real event handler.
    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
    {
      static_cast<UsbWrapper*>(this_ptr)->hotplugAttachEvent(dev);
    }
    else
    {
      static_cast<UsbWrapper*>(this_ptr)->hotplugDetachEvent(dev);
    }

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

  // Check to see if the device is already connected.
  checkForDeviceAlreadyConnected();
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

void UsbWrapper::checkForDeviceAlreadyConnected()
{
  libusb_device **list;
  ssize_t cnt = libusb_get_device_list(context_.get(), &list);

  if (cnt < 0)
  {
    return;
  }

  for (ssize_t i = 0; i < cnt; i++)
  {
    libusb_device *device = list[i];

    libusb_device_descriptor desc;
    if ((libusb_get_device_descriptor(device, &desc) == 0) && (desc.idVendor == kUsbVendorId) &&
      (desc.idProduct == kUsbProductId))
    {
      hotplugAttachEvent(device);
      break;
    }
  }

  libusb_free_device_list(list, 1);
}

void UsbWrapper::hotplugAttachEvent(libusb_device* dev)
{
  if (dev == nullptr)
  {
    return;
  }

  libusb_device_descriptor desc;
  if (libusb_get_device_descriptor(dev, &desc) != LIBUSB_SUCCESS)
  {
    CANTALOUPE_ERROR("Failed to get device descriptor.");
    return;
  }

  // Make sure we have one configuration.
  if (desc.bNumConfigurations < kExpectedConfigurationIndex + 1)
  {
    CANTALOUPE_ERROR("Expected at least {} configuration(s) but received only {}.", kExpectedConfigurationIndex + 1,
      desc.bNumConfigurations);
    return;
  }

  {
    std::lock_guard<std::mutex> lock(device_handle_mutex_);

    libusb_device_handle* handle = nullptr;
    if (libusb_open(dev, &handle) != LIBUSB_SUCCESS)
    {
      CANTALOUPE_ERROR("Failed to open device.");
      return;
    }

    device_handle_.reset(handle);

    // We already checked that there was one configuration previously. Now claim it.
    if (libusb_claim_interface(handle, kExpectedConfigurationIndex) != LIBUSB_SUCCESS)
    {
      CANTALOUPE_ERROR("Failed to claim interface.");
    }
  }

  CANTALOUPE_INFO("Connected!");
}

void UsbWrapper::hotplugDetachEvent(libusb_device* dev)
{
  if (dev == nullptr)
  {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(device_handle_mutex_);
    device_handle_.reset();
  }

  CANTALOUPE_INFO("Disconnected.");
}

bool UsbWrapper::receiveBulkData(uint8_t* data, size_t num_bytes, size_t* actual_num_bytes)
{
  int signed_actual_length = 0;

  {
    std::lock_guard<std::mutex> lock(device_handle_mutex_);
    if (device_handle_ == nullptr)
    {
      CANTALOUPE_ERROR("Invalid device handle.");
      actual_num_bytes = 0;
      return false;
    }

    int retcode = libusb_bulk_transfer(device_handle_.get(), kExpectedEndpointInIdx | LIBUSB_ENDPOINT_IN, data,
      static_cast<int>(num_bytes), &signed_actual_length, kBulkTransferTimeoutMs);

    if (retcode != LIBUSB_SUCCESS)
    {
      CANTALOUPE_ERROR("Failed to initiate transfer (ret = {}).", retcode);
      actual_num_bytes = 0;
      return false;
    }
  }

  *actual_num_bytes = static_cast<size_t>(signed_actual_length);
  return true;
}

bool UsbWrapper::transmitBulkData(uint8_t* data, size_t num_bytes)
{
  int signed_actual_length = 0;

  {
    std::lock_guard<std::mutex> lock(device_handle_mutex_);
    if (device_handle_ == nullptr)
    {
      CANTALOUPE_ERROR("Invalid device handle.");
      return false;
    }

    int retcode = libusb_bulk_transfer(device_handle_.get(), kExpectedEndpointOutIdx | LIBUSB_ENDPOINT_OUT, data,
      static_cast<int>(num_bytes), &signed_actual_length, kBulkTransferTimeoutMs);

    if (retcode != LIBUSB_SUCCESS)
    {
      CANTALOUPE_ERROR("Failed to initiate transfer (ret = {}).", retcode);
      return false;
    }
  }

  return static_cast<size_t>(signed_actual_length) == num_bytes;
}

bool UsbWrapper::transmitControl(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint8_t* data,
  size_t length)
{
  request_type = LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT;

  {
    std::lock_guard<std::mutex> lock(device_handle_mutex_);
    if (device_handle_ == nullptr)
    {
      CANTALOUPE_ERROR("Invalid device handle.");
      return false;
    }

    int num_bytes_tx = libusb_control_transfer(device_handle_.get(), request_type, request, value, index, data,
      static_cast<uint16_t>(length), kBulkTransferTimeoutMs);

    if (num_bytes_tx <= 0)
    {
      CANTALOUPE_ERROR("Failed to transfer control (ret = {}).", num_bytes_tx);
      return false;
    }
  }

  return true;
}

}  // namespace cantaloupe
