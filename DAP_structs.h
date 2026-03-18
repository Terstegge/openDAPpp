//////////////////////////////////////////////////////
//   This file is part of openDAP++, a C++ based
//   implementation of the CMSIS DAP protocol.
//   https://github.com/Terstegge/openDAPpp.git
//
//   (c) A. Terstegge (Andreas.Terstegge@gmail.com)
//////////////////////////////////////////////////////
//
#ifndef DAP_STRUCTS_H
#define DAP_STRUCTS_H

#include <cstdint>

// DAP Commands
enum dap_cmt_t {
    CMD_INFO                = 0x00,
    CMD_HOST_STATUS         = 0x01,
    CMD_CONNECT             = 0x02,
    CMD_DISCONNECT          = 0x03,

    CMD_TRANSFER_CONFIGURE  = 0x04,
    CMD_TRANSFER            = 0x05,
    CMD_TRANSFER_BLOCK      = 0x06,
    CMD_TRANSFER_ABORT      = 0x07,

    CMD_WRITE_ABORT         = 0x08,
    CMD_DELAY               = 0x09,
    CMD_RESET_TARGET        = 0x0a,

    CMD_SWJ_PINS            = 0x10,
    CMD_SWJ_CLOCK           = 0x11,
    CMD_SWJ_SEQUENCE        = 0x12,

    CMD_SWD_CONFIGURE       = 0x13,
    CMD_SWD_SEQUENCE        = 0x1d,

    CMD_JTAG_SEQUENCE       = 0x14,
    CMD_JTAG_CONFIGURE      = 0x15,
    CMD_JTAG_IDCODE         = 0x16,

    CMD_SWO_TRANSPORT       = 0x17,
    CMD_SWO_MODE            = 0x18,
    CMD_SWO_BAUDRATE        = 0x19,
    CMD_SWO_CONTROL         = 0x1a,
    CMD_SWO_STATUS          = 0x1b,
    CMD_SWO_EXT_STATUS      = 0x1e,
    CMD_SWO_DATA            = 0x1c,

    CMD_UART_TRANSPORT      = 0x1f,
    CMD_UART_CONFIGURE      = 0x20,
    CMD_UART_TRANSFER       = 0x21,
    CMD_UART_CONTROL        = 0x22,
    CMD_UART_STATUS         = 0x23,

    CMD_QUEUE_COMMANDS      = 0x7e,
    CMD_EXECUTE_COMMANDS    = 0x7f,

    CMD_VENDOR_0            = 0x80,
    CMD_VENDOR_31           = 0x9f,

    CMD_INVALID             = 0xff,
};

// DAP response status
enum dap_status_t {
    STATUS_OK               = 0x00,
    STATUS_ERROR            = 0xff,
};

// DAP info index
enum info_index_t {
    INFO_VENDOR_NAME        = 0x01,
    INFO_PRODUCT_NAME       = 0x02,
    INFO_SERIAL_NUMBER      = 0x03,
    INFO_CMSIS_DAP_VERSION  = 0x04,
    INFO_DEVICE_VENDOR      = 0x05,
    INFO_DEVICE_NAME        = 0x06,
    INFO_BOARD_VENDOR       = 0x07,
    INFO_BOARD_NAME         = 0x08,
    INFO_FIRMWARE_VERSION   = 0x09,
    INFO_CAPABILITIES       = 0xf0,
    INFO_TEST_DOMAIN_TIMER  = 0xf1,
    INFO_UART_RX_SIZE       = 0xfb,
    INFO_UART_TX_SIZE       = 0xfc,
    INFO_SWO_BUFFER_SIZE    = 0xfd,
    INFO_MAX_PACKET_COUNT   = 0xfe,
    INFO_MAX_PACKET_SIZE    = 0xff,
};

// Info Capabilities structure
union __attribute__((__packed__)) capabilities_t {
    struct __attribute__((__packed__)) {
        // Info 0
        uint8_t swd_support         : 1;
        uint8_t jtag_support        : 1;
        uint8_t swo_uart            : 1;
        uint8_t swo_manchester      : 1;
        uint8_t atomic_commands     : 1;
        uint8_t test_domain_timer   : 1;
        uint8_t swo_streaming_trace : 1;
        uint8_t uart_com_port       : 1;
        // Info 1
        uint8_t usb_com_port        : 1;
        uint8_t                     : 7;
    };
    struct __attribute__((__packed__)) {
        uint8_t info0 {0};
        uint8_t info1 {0};
    };
};
static_assert(sizeof(capabilities_t) == 2);

enum port_t {
    PORT_DEFAULT    = 0,
    PORT_SWD        = 1,
    PORT_JTAG       = 2,
};

// Transfer request type
union __attribute__((__packed__)) transfer_request_t {
    struct __attribute__((__packed__)) {
        uint8_t access_port     : 1;
        uint8_t read            : 1;
        uint8_t addr_a2         : 1;
        uint8_t addr_a3         : 1;
        uint8_t match_value     : 1;
        uint8_t match_mask      : 1;
        uint8_t                 : 1;
        uint8_t time_stamp      : 1;
    };
    uint8_t value {0};
};
static_assert(sizeof(transfer_request_t) == 1);

constexpr transfer_request_t DP_READ_RDBUFF = {
        .access_port = 0,
        .read        = 1,
        .addr_a2     = 1,
        .addr_a3     = 1,
        .match_value = 0,
        .match_mask  = 0,
        .time_stamp  = 0
};

constexpr transfer_request_t DP_WRITE_ABORT = {
        .access_port = 0,
        .read        = 0,
        .addr_a2     = 0,
        .addr_a3     = 0,
        .match_value = 0,
        .match_mask  = 0,
        .time_stamp  = 0
};

enum class ack_t : uint8_t {
    invalid     = 0,
    okay        = 1,
    wait        = 2,
    fault       = 4,
    no_ack      = 7
};

// Transfer response type
union __attribute__((__packed__)) transfer_response_t {
    struct __attribute__((__packed__)) {
        ack_t ack               : 3;
        uint8_t protocol_error  : 1;
        uint8_t value_mismatch  : 1;
        uint8_t                 : 3;
    };
    uint8_t value {0};
};
static_assert(sizeof(transfer_response_t) == 1);

// SWD configuration
union __attribute__((__packed__)) swd_config_t {
    struct __attribute__((__packed__)) {
        uint8_t turnaround      : 2;
        uint8_t data_phase      : 1;
        uint8_t                 : 5;
    };
    uint8_t value {0};
};
static_assert(sizeof(swd_config_t) == 1);

// SWD sequence info
union __attribute__((__packed__)) swd_sequence_info_t {
    struct __attribute__((__packed__)) {
        uint8_t cycles          : 6;
        uint8_t                 : 1;
        uint8_t read            : 1;
    };
    uint8_t value {0};
};
static_assert(sizeof(swd_sequence_info_t) == 1);

// JTAG sequence info
union __attribute__((__packed__)) jtag_sequence_info_t {
    struct __attribute__((__packed__)) {
        uint8_t cycles          : 6;
        uint8_t tms_value       : 1;
        uint8_t tdo_capture     : 1;
    };
    uint8_t value {0};
};
static_assert(sizeof(jtag_sequence_info_t) == 1);

// SWJ Pins mapping
union __attribute__((__packed__)) pin_mappings_t {
    struct __attribute__((__packed__)) {
        uint8_t SWCLK_TCK       : 1;
        uint8_t SWDIO_TMS       : 1;
        uint8_t TDI             : 1;
        uint8_t TDO             : 1;
        uint8_t                 : 1;
        uint8_t nTRST           : 1;
        uint8_t                 : 1;
        uint8_t nRESET          : 1;
    };
    uint8_t value {0};
};
static_assert(sizeof(pin_mappings_t) == 1);


// SWD Header structure
union __attribute__((__packed__)) swd_header_t {
    struct __attribute__((__packed__)) {
        uint8_t start           : 1;
        uint8_t access_port     : 1;
        uint8_t read            : 1;
        uint8_t addr_a2         : 1;
        uint8_t addr_a3         : 1;
        uint8_t parity          : 1;
        uint8_t stop            : 1;
        uint8_t park            : 1;
    };
    uint8_t value {0};
};
static_assert(sizeof(swd_header_t) == 1);

// Struct for a single USB Packet
// including the data size.
struct usb_buf_t {
    uint8_t len {0};
    uint8_t data[USB_DEFAULT_PAKET_SIZE] {0};
};

// Return type reporting consumed size and the
// response size.
struct ret_t {
    uint16_t request_consumed;
    uint16_t response_size;
};

enum {
    SWD_DP_R_IDCODE           = 0x00,
    SWD_DP_W_ABORT            = 0x00,
    SWD_DP_R_RDBUFF           = 0x0c,
};

enum {
    JTAG_ABORT                = 0x08,
    JTAG_DPACC                = 0x0a,
    JTAG_APACC                = 0x0b,
    JTAG_IDCODE               = 0x0e,
    JTAG_BYPASS               = 0x0f,
    JTAG_INVALID              = 0xff,
};

//    DAP_TRANSFER_JTAG_ABORT   = 1 << 16,

#endif // DAP_STRUCTS_H
