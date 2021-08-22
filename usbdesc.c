// Copyright 2017 by Robert Evans (rrevans@gmail.com)
//
// This file is part of ubaboot.
//
// ubaboot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ubaboot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ubaboot.  If not, see <http://www.gnu.org/licenses/>.

#include <avr/pgmspace.h>
#include <stdint.h>
#include "config.h"
#include "packet.h"

#if !defined(VENDOR_ID) || !defined(PRODUCT_ID)
#error config error: you must setup USB vendor/device IDs
#endif

#define USB_DESCRIPTOR_TYPE_DEVICE          1
#define USB_DESCRIPTOR_TYPE_CONFIGURATION   2
#define USB_DESCRIPTOR_TYPE_INTERFACE       4
#define USB_DESCRIPTOR_TYPE_ENDPOINT        5
#define USB_DESCRIPTOR_TYPE_HID             0x21
#define USB_DESCRIPTOR_TYPE_HID_REPORT      0x22
#define USB_BCD_VERSION_11                  0x0110

#define USBDESC(x) __attribute__((section(".usbdesc." x), used))

struct USB_DeviceDescriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubClass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize0;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t iManufacturer;
  uint8_t iProduct;
  uint8_t iSerialNumber;
  uint8_t bNumConfigurations;
} __attribute__((packed));

const struct USB_DeviceDescriptor dev_desc USBDESC("device") = {
  .bLength = sizeof(struct USB_DeviceDescriptor),
  .bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
  .bcdUSB = USB_BCD_VERSION_11,
  .bDeviceClass = 0,
  .bDeviceSubClass = 0,
  .bDeviceProtocol = 0,
  .bMaxPacketSize0 = MAX_PACKET,
  .idVendor = VENDOR_ID,
  .idProduct = PRODUCT_ID,
  .bcdDevice = 0x0050,
  .bNumConfigurations = 1,
};

struct USB_ConfigurationDescriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t MaxPower;
} __attribute__((packed));

struct USB_InterfaceDescriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
} __attribute__((packed));

struct USB_HIDDescriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdHID;
  uint8_t bCountryCode;
  uint8_t bNumReportDescriptors;
  uint8_t bReportDescriptorType;
  uint16_t wReportDescriptorLength;
} __attribute__((packed));

struct USB_EndpointDescriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t bInterval;
} __attribute__((packed));

/* The part of configuration descriptor set before the HID descriptor. */
struct USB_ConfigurationDescriptorSet_1 {
  struct USB_ConfigurationDescriptor config;
  struct USB_InterfaceDescriptor ac_interface;
} __attribute__((packed));

/* The complete configuration descriptor set. */
struct USB_ConfigurationDescriptorSet {
  struct USB_ConfigurationDescriptorSet_1 first_part;
  struct USB_HIDDescriptor hid;
  struct USB_EndpointDescriptor hid_input_ep;
} __attribute__((packed));

const uint8_t hid_report_desc[] USBDESC("hid_report") = {
  0x06, 0xDC, 0xFF,  // Usage Page (Vendor Defined 0xDCFF)
  0x09, 0xFB,        // Usage (0xFB)
  0xA1, 0x01,        // Collection (Application)
  0x09, 0x02,        //   Usage (0x02)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0xFF,        //   Logical Maximum (255)
  0x75, 0x08,        //   Report Size (8)
  0x96, 0x82, 0x00,  //   Report Count (130) -> SPM_PAGESIZE (128 bytes) + 2
  0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  0xC0,              // End Collection
};

const struct USB_ConfigurationDescriptorSet_1 conf_desc USBDESC("config_1") = {
  .config = {
    .bLength = sizeof(struct USB_ConfigurationDescriptor),
    .bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION,
    .wTotalLength = sizeof(struct USB_ConfigurationDescriptorSet),
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .bmAttributes = 0x80,
    .MaxPower = 50,  // 100mA
  },
  .ac_interface = {
    .bLength = sizeof(struct USB_InterfaceDescriptor),
    .bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = 3,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
  },
};

const struct USB_HIDDescriptor hid_desc USBDESC("hid") = {
  .bLength = sizeof(struct USB_HIDDescriptor),
  .bDescriptorType = USB_DESCRIPTOR_TYPE_HID,
  .bcdHID = 0x0111,
  .bCountryCode = 0,
  .bNumReportDescriptors = 1,
  .bReportDescriptorType = USB_DESCRIPTOR_TYPE_HID_REPORT,
  .wReportDescriptorLength = sizeof(hid_report_desc),
};

const struct USB_EndpointDescriptor hid_input_ep_desc USBDESC("hid_input_ep") = {
  .bLength = sizeof(struct USB_EndpointDescriptor),
  .bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
  .bEndpointAddress = 0x81,
  .bmAttributes = 0x03,
  .wMaxPacketSize = MAX_PACKET,
  .bInterval = 5,
};
