/*
 *  Seek device interface
 *  Author: Maarten Vandersteegen
 */

#include "SeekDevice.h"
#include "SeekLogging.h"
#include <libusb.h>
#include <libudev.h>
#include <endian.h>
#include <stdio.h>
#include <cstring>

using namespace LibSeek;

SeekDevice::SeekDevice(int vendor_id, int product_id, std::string dev_filename, int timeout) :
    m_vendor_id(vendor_id),
    m_product_id(product_id),
    m_dev_filename(dev_filename),
    m_timeout(timeout),
    m_is_opened(false),
    m_ctx(nullptr),
    m_handle(nullptr) { }

SeekDevice::~SeekDevice()
{
    close();
};

bool SeekDevice::open()
{
    int res;
    int bConfigurationValue;

    if (m_handle != NULL) {
        error("Error: SeekDevice already opened\n");
        return false;
    }

    //libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_WARNING);

    // Init libusb
    res = libusb_init(&m_ctx);
    if (res < 0) {
        error("Error: libusb init failed: %s\n", libusb_error_name(res));
        return false;
    }

    if (!open_device()) {
        close();
        return false;
    }

    res = libusb_get_configuration(m_handle, &bConfigurationValue);
    if (res != 0) {
        error("Error: libusb get configuration failed: %s\n", libusb_error_name(res));
        close();
        return false;
    }
    debug("bConfigurationValue : %d\n", bConfigurationValue);

    if (bConfigurationValue != 1) {
        res = libusb_set_configuration(m_handle, 1);
        if (res != 0) {
            error("Error: libusb set configuration failed: %s\n", libusb_error_name(res));
            close();
            return false;
        }
    }

    res = libusb_claim_interface(m_handle, 0);
    if (res < 0) {
        error("Error: failed to claim interface: %s\n", libusb_error_name(res));
        close();
        return false;
    }

    m_is_opened = true;
    return true;
}

void SeekDevice::close()
{
    if (m_handle != NULL) {
        libusb_release_interface(m_handle, 0);  /* release claim */
        libusb_close(m_handle);                 /* revert open */
        m_handle = NULL;
    }

    if (m_ctx != NULL) {
        libusb_exit(m_ctx);                     /* revert exit */
        m_ctx = NULL;
    }

    m_is_opened = false;
}

bool SeekDevice::isOpened()
{
    return m_is_opened;
}

bool SeekDevice::request_set(DeviceCommand::Enum command, std::vector<uint8_t>& data)
{
    return control_transfer(0, static_cast<char>(command), 0, 0, data);
}

bool SeekDevice::request_get(DeviceCommand::Enum command, std::vector<uint8_t>& data)
{
    return control_transfer(1, static_cast<char>(command), 0, 0, data);
}

bool SeekDevice::fetch_frame(uint16_t* buffer, std::size_t size, std::size_t request_size)
{
    int res;
    int actual_length;
    int todo = size * sizeof(uint16_t);
    uint8_t* buf = reinterpret_cast<uint8_t*>(buffer);
    int done = 0;

    while (todo != 0) {
        debug("Asking for %d B of data at %d\n", request_size, done);
        res = libusb_bulk_transfer(m_handle, 0x81, &buf[done], request_size, &actual_length, m_timeout);
        if (res == LIBUSB_ERROR_TIMEOUT)
        {
            error("Error: LIBUSB_ERROR_TIMEOUT\n");
        } else if (res != 0) {
            error("Error: bulk transfer failed: %s\n", libusb_error_name(res));
            return false;
        }
        debug("Actual length %d\n", actual_length);
        todo -= actual_length;
        done += actual_length;
    }
    correct_endianness(buffer, size);

    return true;
}

std::string get_realpath(std::string path) {
    char *real_path = realpath(path.c_str(), NULL);
    if (!real_path) { // or: if (res != NULL)
        return std::string();
    }
    std::string str(real_path);
    return str;
}

bool SeekDevice::open_device()
{
    int res;
    int idx_dev;
    int cnt;
    bool found = false;
    struct libusb_device **devs;
    struct libusb_device_descriptor desc;

    int bus, device;
    bool match_bd = false;
    debug("input devpath [%d] %s\n", m_dev_filename.length(), m_dev_filename.c_str());
    if (m_dev_filename.length() > 0) {
        // Device provided

        std::string devpath = get_realpath(m_dev_filename);
        debug("input real devpath [%d] %s\n", devpath.length(), devpath.c_str());

        if (devpath.length() <= 0) {
            error("Error: device invalid: %s\n", m_dev_filename.c_str());
            return false;
        }

        int n = sscanf(devpath.c_str(), "/dev/bus/usb/%d/%d", &bus, &device);

        if (n != 2) {
            error("Error: not a /dev/bus/usb device: %s\n", devpath.c_str());
            return false;
        }

        // Succeeded
        debug("parsed bus/device %d/%d\n", bus, device);

        match_bd = true;
    }

    cnt = libusb_get_device_list(m_ctx, &devs);
    if (cnt < 0) {
        error("Error: no devices found: %s\n", libusb_error_name(cnt));
        return false;
    }

    debug("Device Count : %d\n", cnt);

    for (idx_dev = 0; idx_dev < cnt; idx_dev++) {
        res = libusb_get_device_descriptor(devs[idx_dev], &desc);
        if (res < 0) {
            libusb_free_device_list(devs, 1);
            error("Error: failed to get device descriptor: %s\n", libusb_error_name(res));
            return false;
        }

        debug("vendor: %x  product: %x\n", desc.idVendor, desc.idProduct);

        debug("desc: bLength %d, bDescriptorType %d, bcdUSB %d, bDeviceClass %d, bDeviceSubClass %d, bDeviceProtocol %d, bMaxPacketSize0 %d, idVendor %d, idProduct %d, bcdDevice %d, iManufacturer %d, iProduct %d, iSerialNumber %d, bNumConfigurations %d\n", desc.bLength, desc.bDescriptorType, desc.bcdUSB, desc.bDeviceClass, desc.bDeviceSubClass, desc.bDeviceProtocol, desc.bMaxPacketSize0, desc.idVendor, desc.idProduct, desc.bcdDevice, desc.iManufacturer, desc.iProduct, desc.iSerialNumber, desc.bNumConfigurations);
        uint8_t path[255];
        res = libusb_get_port_numbers(devs[idx_dev], path, 255);
        if (res < 0) {
          error("failed to get path: %s\n", libusb_error_name(res));
        } else {
          debug("path: ");
          for (int i = 0; i < res; i++) {
            printf("%d, ", path[i]);
          }
          printf(";\n");
        }
        res = libusb_get_bus_number(devs[idx_dev]);
        if (res < 0) {
          error("failed to get bus: %s\n", libusb_error_name(res));
        } else {
          debug("bus: %d\n", res);
        }
        if (match_bd && bus != res) {
          error("did not match bus\n");
          continue;
        }
        debug("!!matched bus\n");
        res = libusb_get_port_number(devs[idx_dev]);
        if (res < 0) {
          error("failed to get port: %s\n", libusb_error_name(res));
        } else {
          debug("port: %d\n", res);
        }
        res = libusb_get_device_address(devs[idx_dev]);
        if (res < 0) {
          error("failed to get address: %s\n", libusb_error_name(res));
        } else {
          debug("address: %d\n", res);
        }
        if (match_bd && device != res) {
          error("did not match addr\n");
          continue;
        }
        debug("!!matched address\n");
        if (desc.idVendor == m_vendor_id && desc.idProduct == m_product_id) {
          debug("!!matched vendor/product\n");
          found = true;
          break;
        } else {
          error("did not match vendor/product\n");
          continue;
        }
    }

    if (!found) {
        libusb_free_device_list(devs, 1);
        error("Error: Did not found device %04x:%04x\n", m_vendor_id, m_product_id);
        return false;
    }

    res = libusb_open(devs[idx_dev], &m_handle);
    libusb_free_device_list(devs, 1);

    if (res < 0) {
        error("Error: libusb init failed: %s\n", libusb_error_name(res));
        return false;
    }

    return true;
}


bool SeekDevice::control_transfer(bool direction, uint8_t req, uint16_t value, uint16_t index, std::vector<uint8_t>& data)
{
    int res;
    uint8_t bmRequestType = (direction ? LIBUSB_ENDPOINT_IN : LIBUSB_ENDPOINT_OUT)
                            | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE;
    if (data.size() == 0) {
        data.reserve(16);
    }
    
    uint8_t* aData = data.data();
    uint16_t wLength = data.size();

    // to device
    debug("ctrl_transfer to/from dev(0x%x, 0x%x, 0x%x, 0x%x, %d)\n",
                    bmRequestType, req, value, index, wLength);

    res = libusb_control_transfer(m_handle, bmRequestType, req, value, index, aData, wLength, m_timeout);

    if (res < 0) {
        error("Error: control transfer failed: %s\n", libusb_error_name(res));
        return false;
    }

    if (res != wLength) {
        error("Error: control transfer returned %d bytes, expected %d bytes\n", res, wLength);
        return false;
    }

    return true;
}

void SeekDevice::correct_endianness(uint16_t* buffer, std::size_t size)
{
    std::size_t i;

    for (i=0; i<size; i++) {
        buffer[i] = le16toh(buffer[i]);
    }
}
