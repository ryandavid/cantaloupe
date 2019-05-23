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
#ifndef GS_USB_WRAPPER_H_
#define GS_USB_WRAPPER_H_

#include <cantaloupe/can_frame.h>
#include <cantaloupe/libusb_forward_declare.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

namespace cantaloupe
{

// Deleter for the smart pointer that owns the LibUSB context.
struct libUsbContextDeleter
{
  void operator()(libusb_context* context) { libusb_exit(context); }
};

// Deleter for the LibUSB device handle.  Cheat by passing in the configuration index as a template arg.
template<int ConfigurationIndex>
struct libUsbDeviceHandleDeleter
{
  void operator()(libusb_device_handle* handle)
  {
    libusb_release_interface(handle, ConfigurationIndex);
    libusb_close(handle);
  }
};

// Types used for control messages.
enum class ControlType
{
  IN,
  OUT
};

class GsUsbWrapper
{
 public:
  // USB Vendor and Product IDs we want to attach to.
  static constexpr uint16_t kUsbVendorId = 0x1d50;
  static constexpr uint16_t kUsbProductId = 0x606f;

  // Timeout in microseconds to use when processing hotplug events.
  static constexpr uint32_t kHotplugEventTimeoutUSecs = 100 * 1000;

  // Index of the USB device configuration we want to attach to.
  static constexpr uint8_t kExpectedConfigurationIndex = 0;
  static constexpr uint8_t kExpectedEndpointInIdx = 1;  // To Host from USB device.
  static constexpr uint8_t kExpectedEndpointOutIdx = 2;  // To USB device from Host.

  // Default time (ms) to wait for a control transfer to succeed.
  static constexpr uint32_t kDefaultControlTransferTimeoutMs = 100;

  GsUsbWrapper();
  ~GsUsbWrapper();

  // Get the version of LibUSB as a string.
  static const char* getLibUSBVersionString();

  // Determine if the device is currently connected.
  bool isConnected();

  // Turn on/off the identify LEDs.
  bool setIdentifyLeds(bool enable_identify_leds);

  // Enable the CAN channel.  Optionally enable loopback mode.
  bool startChannel(bool loopback = false);

  // Disable the CAN channel.
  bool stopChannel();

  // Set the bitrate for the channel.
  bool setBitrate(uint32_t bitrate);

  // Write a single CAN frame to the bus.  Optionally specify a timeout in ms, or default to zero for blocking.
  bool writeCanFrame(const CanFrame& frame, uint32_t timeout_ms = 0);

  // Rear a single CAN frame to the bus.  Optionally specify a timeout in ms, or default to zero for blocking.
  bool readCanFrame(CanFrame* frame, uint32_t timeout_ms = 0);

 private:
  // Determine if the device is already present at startup.
  void checkForDeviceAlreadyConnected();

  // Callbacks used with the hotplug events.
  void hotplugAttachEvent(libusb_device* dev);
  void hotplugDetachEvent(libusb_device* dev);

  // Thread responsible for kicking LibUSB.
  void hotplugMonitorThread() const;

  // Receive data from the bulk endpoint.
  bool receiveBulkData(void* data, size_t num_bytes, size_t* actual_num_bytes, uint32_t timeout_ms);

  // Transmit data on the bulk endpoint.
  bool transmitBulkData(void* data, size_t num_bytes, uint32_t timeout_ms);

  // Transmit a control message on the interface.
  bool transmitControl(ControlType type, uint8_t request, uint16_t value, uint16_t index, void* data, size_t length);

  // Set the host format on the device.
  bool setHostFormat();

  // The LibUSB context.
  std::unique_ptr<libusb_context, libUsbContextDeleter> context_;

  // Handle to be used with LibUSB hotplug events.
  int hotplug_handle_;

  // Flag indicating that the hotplug envent thread should terminate.
  bool hotplug_thread_shutdown_;

  // The thread responsible for kicking LibUSB.
  std::thread hotplug_thread_;

  // The LibUSB device handle, and a mutex protecting it.
  std::mutex device_handle_mutex_;
  std::unique_ptr<libusb_device_handle, libUsbDeviceHandleDeleter<kExpectedConfigurationIndex>> device_handle_;
};

}  // namespace cantaloupe

#endif  // GS_USB_WRAPPER_H_
