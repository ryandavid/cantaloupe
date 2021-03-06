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
#include <cantaloupe/gs_usb_commands.h>
#include <cantaloupe/gs_usb_wrapper.h>
#include <cantaloupe/log.h>

#include <algorithm>
#include <functional>
#include <stdexcept>

#include <libusb.h>

// We expect the symbol LIBUSB_VERSION_STRING to be set by CMake with the version number.  This is a lazy way of letting
// us get the version number without having to call into LibUSB's API and assemble the string ourselves.
#ifndef LIBUSB_VERSION_STRING
#warning LIBUSB_VERSION_STRING is not set.
#endif

namespace cantaloupe
{

GsUsbWrapper::GsUsbWrapper() :
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
      static_cast<GsUsbWrapper*>(this_ptr)->hotplugAttachEvent(dev);
    }
    else
    {
      static_cast<GsUsbWrapper*>(this_ptr)->hotplugDetachEvent(dev);
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
  hotplug_thread_ = std::thread(std::bind(&GsUsbWrapper::hotplugMonitorThread, this));

  // Check to see if the device is already connected.
  checkForDeviceAlreadyConnected();
}

GsUsbWrapper::~GsUsbWrapper()
{
  // If we have an open channel, close it.
  stopChannel();

  // If the hotplug monitor thread is still running, signal it to die and then wait for it.
  if (hotplug_thread_.joinable() == true)
  {
    hotplug_thread_shutdown_ = true;
    hotplug_thread_.join();
  }

  // Deregister the hotplug callback.
  libusb_hotplug_deregister_callback(context_.get(), hotplug_handle_);
}

const char* GsUsbWrapper::getLibUSBVersionString()
{
  return LIBUSB_VERSION_STRING;
}

bool GsUsbWrapper::isConnected()
{
  std::lock_guard<std::mutex> lock(device_handle_mutex_);
  return device_handle_ != nullptr;
}

void GsUsbWrapper::hotplugMonitorThread() const
{
  while (hotplug_thread_shutdown_ == false)
  {
    // Use a timeout so that way there is a opportunity for this thread to be signaled to be closed.
    timeval duration;
    duration.tv_sec = 0;
    duration.tv_usec = kHotplugEventTimeoutUSecs;

    libusb_handle_events_timeout(context_.get(), &duration);
  }
}

void GsUsbWrapper::checkForDeviceAlreadyConnected()
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

void GsUsbWrapper::hotplugAttachEvent(libusb_device* dev)
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

  setHostFormat();
  CANTALOUPE_INFO("Connected!");
}

void GsUsbWrapper::hotplugDetachEvent(libusb_device* dev)
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

bool GsUsbWrapper::receiveBulkData(void* data, size_t num_bytes, size_t* actual_num_bytes, uint32_t timeout_ms)
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

    int retcode = libusb_bulk_transfer(device_handle_.get(), kExpectedEndpointInIdx | LIBUSB_ENDPOINT_IN,
      static_cast<uint8_t*>(data), static_cast<int>(num_bytes), &signed_actual_length, timeout_ms);

    if (retcode != LIBUSB_SUCCESS)
    {
      // Only complain if the error was not a timeout.
      if (retcode != LIBUSB_ERROR_TIMEOUT)
      {
        CANTALOUPE_ERROR("Failed to initiate transfer (ret = {}: {}).", retcode, libusb_error_name(retcode));
      }

      actual_num_bytes = 0;
      return false;
    }
  }

  *actual_num_bytes = static_cast<size_t>(signed_actual_length);
  return true;
}

bool GsUsbWrapper::transmitBulkData(void* data, size_t num_bytes, uint32_t timeout_ms)
{
  int signed_actual_length = 0;

  {
    std::lock_guard<std::mutex> lock(device_handle_mutex_);
    if (device_handle_ == nullptr)
    {
      CANTALOUPE_ERROR("Invalid device handle.");
      return false;
    }

    int retcode = libusb_bulk_transfer(device_handle_.get(), kExpectedEndpointOutIdx | LIBUSB_ENDPOINT_OUT,
      static_cast<uint8_t*>(data), static_cast<int>(num_bytes), &signed_actual_length, timeout_ms);

    if (retcode != LIBUSB_SUCCESS)
    {
      CANTALOUPE_ERROR("Failed to initiate transfer (ret = {}).", retcode);
      return false;
    }
  }

  return static_cast<size_t>(signed_actual_length) == num_bytes;
}

bool GsUsbWrapper::transmitControl(ControlType type, uint8_t request, uint16_t value, uint16_t index, void* data,
  size_t length)
{
  uint8_t request_type = LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_VENDOR;
  request_type |= (type == ControlType::OUT) ? LIBUSB_ENDPOINT_OUT : LIBUSB_ENDPOINT_IN;

  {
    std::lock_guard<std::mutex> lock(device_handle_mutex_);
    if (device_handle_ == nullptr)
    {
      CANTALOUPE_ERROR("Invalid device handle.");
      return false;
    }

    int num_bytes_tx = libusb_control_transfer(device_handle_.get(), request_type, request, value, index,
      static_cast<uint8_t*>(data), static_cast<uint16_t>(length), kDefaultControlTransferTimeoutMs);

    if (num_bytes_tx <= 0)
    {
      CANTALOUPE_ERROR("Failed to transfer control (ret = {}).", num_bytes_tx);
      return false;
    }
  }

  return true;
}

bool GsUsbWrapper::setIdentifyLeds(bool enable_identify_leds)
{
  uint32_t enable_typed = enable_identify_leds;
  return transmitControl(ControlType::OUT, GsUsbBreq::IDENTIFY, 0, 0, &enable_typed, sizeof(enable_typed));
}

bool GsUsbWrapper::setHostFormat()
{
  // We are expected to send a magic packet to the device so it can determine our endianess.  Default constructor
  // for GsHostConfig does all the work for us.
  GsHostConfig config;
  return transmitControl(ControlType::OUT, GsUsbBreq::HOST_FORMAT, 0, 0, &config, sizeof(config));
}

bool GsUsbWrapper::startChannel(bool loopback)
{
  GsDeviceMode device_mode;
  device_mode.mode = GsDeviceMode::kModeStart;
  device_mode.flags = GsDeviceMode::kFlagHwTimestamp;

  if (loopback == true)
  {
    device_mode.flags |= GsDeviceMode::kFlagLoopBack;
  }

  return transmitControl(ControlType::OUT, GsUsbBreq::MODE, 0, 0, &device_mode, sizeof(device_mode));
}

bool GsUsbWrapper::stopChannel()
{
  GsDeviceMode device_mode;
  device_mode.mode = GsDeviceMode::kModeReset;
  device_mode.flags = 0;

  return transmitControl(ControlType::OUT, GsUsbBreq::MODE, 0, 0, &device_mode, sizeof(device_mode));
}

bool GsUsbWrapper::setBitrate(uint32_t bitrate)
{
  // Values borrowed from candleLight_winusbtest ( https://github.com/HubertD/candleLight_winusbtest ).
  GsDeviceBitTiming timing;
  timing.prop_seg = 1;

  timing.prop_seg = 1;
  timing.sjw = 1;
  timing.phase_seg1 = 13 - timing.prop_seg;
  timing.phase_seg2 = 2;

  switch (bitrate)
  {
    case 10000:
      timing.brp = 300;
      break;

    case 20000:
      timing.brp = 150;
      break;

    case 50000:
      timing.brp = 60;
      break;

    case 83333:
      timing.brp = 36;
      break;

    case 100000:
      timing.brp = 30;
      break;

    case 125000:
      timing.brp = 24;
      break;

    case 250000:
      timing.brp = 12;
      break;

    case 500000:
      timing.brp = 6;
      break;

    case 800000:
      timing.brp = 4;
      timing.phase_seg1 = 12 - timing.prop_seg;
      timing.phase_seg2 = 2;
      break;

    case 1000000:
      timing.brp = 3;
      break;

    default:
      return false;
  }

  return transmitControl(ControlType::OUT, GsUsbBreq::BITTIMING, 0, 0, &timing, sizeof(timing));
}

bool GsUsbWrapper::writeCanFrame(const CanFrame& frame, uint32_t timeout_ms )
{
  GsHostCanFrame output;
  output.can_id = frame.id;
  output.echo_id = 0;

  // Make sure that the DLC never exceeds our max data size.
  using dlc_type = decltype(output.can_dlc);
  output.can_dlc = std::min(static_cast<dlc_type>(frame.dlc), static_cast<dlc_type>(CanFrame::kDataNumMaxBytes));

  output.channel = 0;
  output.flags = 0;
  output.reserved = 0;

  // Check at compile time that our CAN frame data lengths match, and then copy all in one fell swoop.
  static_assert(sizeof(output.data) / sizeof(output.data[0]) == CanFrame::kDataNumMaxBytes, "CAN data size mismatch");
  std::copy_n(&frame.data[0], output.can_dlc, &output.data[0]);

  output.timestamp_us = 0;

  return transmitBulkData(&output, sizeof(frame), timeout_ms);
}

bool GsUsbWrapper::readCanFrame(CanFrame* frame, uint32_t timeout_ms)
{
  GsHostCanFrame input;
  size_t actual_num_bytes = 0;

  if (receiveBulkData(&input, sizeof(input), &actual_num_bytes, timeout_ms) == false)
  {
    return false;
  }

  if (actual_num_bytes != sizeof(input))
  {
    return false;
  }

  frame->id = input.can_id;
  frame->error_frame = input.can_id & GsHostCanFrame::kCanIdErrorFlag;
  frame->rtr_frame = input.can_id & GsHostCanFrame::kCanIdRtrFlag;
  frame->eff_frame = input.can_id & GsHostCanFrame::kCanIdEffFlag;

  // We expect that the echo ID be set to uint32_t(-1) when its not loopback.
  frame->from_tx = input.echo_id != GsHostCanFrame::kEchoIdNormalRxFrame;

  // Make sure that the DLC never exceeds our max data size.
  using dlc_type = decltype(frame->dlc);
  frame->dlc = std::min(static_cast<dlc_type>(input.can_dlc), static_cast<dlc_type>(CanFrame::kDataNumMaxBytes));

  // Check at compile time that our CAN frame data lengths match, and then copy all in one fell swoop.
  static_assert(sizeof(input.data) / sizeof(input.data[0]) == CanFrame::kDataNumMaxBytes, "CAN data size mismatch");
  std::copy_n(&input.data[0], frame->dlc, &frame->data[0]);

  frame->timestamp_us = input.timestamp_us;

  return true;
}

}  // namespace cantaloupe
