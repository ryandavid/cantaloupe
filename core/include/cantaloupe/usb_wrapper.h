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
#include <mutex>
#include <string>
#include <thread>

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

class UsbWrapper
{
 public:
  // USB Vendor and Product IDs we want to attach to.
  static constexpr uint16_t kUsbVendorId = 0x1d50;
  static constexpr uint16_t kUsbProductId = 0x606f;

  // Timeout in seconds to use when processing hotplug events.
  static constexpr uint16_t kHotplugEventTimeoutSecs = 1;
  static constexpr int kBulkTransferTimeoutMs = 100;

  // Index of the USB device configuration we want to attach to.
  static constexpr uint8_t kExpectedConfigurationIndex = 0;
  static constexpr uint8_t kExpectedEndpointInIdx = 1;  // To Host from USB device.
  static constexpr uint8_t kExpectedEndpointOutIdx = 2;  // To USB device from Host.

  static constexpr int kReceiveBufferSizeBytes = 1024;

  UsbWrapper();
  ~UsbWrapper();

  bool setIdentifyLeds(bool enable_identify_leds);
  bool startChannel(bool enable, bool loopback = false);
  bool setBitrate(uint32_t bitrate);

 private:
  void checkForDeviceAlreadyConnected();

  void hotplugAttachEvent(libusb_device* dev);
  void hotplugDetachEvent(libusb_device* dev);

  void hotplugMonitorThread() const;

  bool receiveBulkData(uint8_t* data, size_t num_bytes, size_t* actual_num_bytes);
  bool transmitBulkData(uint8_t* data, size_t num_bytes);
  bool transmitControl(ControlType type, uint8_t request, uint16_t value, uint16_t index, void* data, size_t length);

  bool setHostFormat();

  std::unique_ptr<libusb_context, libUsbContextDeleter> context_;
  int hotplug_handle_;

  bool hotplug_thread_shutdown_;
  std::thread hotplug_thread_;

  std::mutex device_handle_mutex_;
  std::unique_ptr<libusb_device_handle, libUsbDeviceHandleDeleter<kExpectedConfigurationIndex>> device_handle_;
};

}  // namespace cantaloupe

#endif  // USB_WRAPPER_H_
