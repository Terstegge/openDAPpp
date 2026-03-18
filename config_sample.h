//////////////////////////////////////////////////////
//   This file is part of openDAP++, a C++ based
//   implementation of the CMSIS DAP protocol.
//   https://github.com/Terstegge/openDAPpp.git
//
//   (c) A. Terstegge (Andreas.Terstegge@gmail.com)
//////////////////////////////////////////////////////
//
#ifndef CONFIG_H
#define CONFIG_H

#define USB_DEFAULT_PAKET_SIZE 64

// CMSIS DAP configuration
//////////////////////////
#define DAP_PROTOCOL_VERSION        "2.1.1"

// Vendor Name and Product Name. If these values are set to empty
// strings, the USB Device Information is used to obtain these values!
#define DAP_VENDOR_NAME             "USB dev manufacturer"
#define DAP_PRODUCT_NAME            "USB dev produt"
#define DAP_FIRMWARE_VERSION        "1.0"

// Device/Board information if debug probe is fixed to a dev board.
// Use empty strings if the chip device and board are not know.
#define DAP_DEVICE_VENDOR           "vendor"
#define DAP_DEVICE_NAME             "device name"
#define DAP_BOARD_VENDOR            "board vendor"
#define DAP_BOARD_NAME              "board name"

// Maximum Package Buffers for Command and Response data.
// This configuration settings is used to optimize the communication
// performance with the debugger and depends on the USB peripheral.
#define DAP_MAX_PACKET_COUNT        10

// Maximum Package Size for Command and Response data.
// This configuration settings is used to optimize the communication
// performance with the debugger and depends on the USB peripheral.
// Typical vales are 64 for Full-speed USB HID or WinUSB,
// 1024 for High-speed USB HID and 512 for High-speed USB WinUSB.
#define DAP_MAX_PACKET_SIZE         USB_DEFAULT_PAKET_SIZE

// CMSIS DAP Protocol support
#define DAP_CAP_SWD_SUPPORT         true
#define DAP_CAP_JTAG_SUPPORT        true
#define DAP_DEFAULT_PORT            PORT_SWD
#define DAP_DEFAULT_CLOCK           1000000

// SWO settings
#define DAP_CAP_SWO_UART            false
#define DAP_CAP_SWO_MANCHESTER      false
#define DAP_CAP_SWO_STREAMING_TRACE false
#define DAP_SWO_BUFFER_SIZE         0

// DAP UART settings
#define DAP_CAP_UART_COM_PORT       false
#define DAP_CAP_USB_COM_PORT        false
#define DAP_UART_RX_SIZE            0
#define DAP_UART_TX_SIZE            0

#define JTAG_DEV_COUNT              8
#define JTAG_IR_LENGTH              4


#endif // CONFIG_H

