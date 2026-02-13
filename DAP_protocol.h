// ---------------------------------------------
//           This file is part of
//      _  _   __    _   _    __    __
//     ( \/ ) /__\  ( )_( )  /__\  (  )
//      \  / /(__)\  ) _ (  /(__)\  )(__
//      (__)(__)(__)(_) (_)(__)(__)(____)
//
//     Yet Another HW Abstraction Library
//      Copyright (C) Andreas Terstegge
//      BSD Licensed (see file LICENSE)
//
// ---------------------------------------------
//
#ifndef DAP_PROTOCOL_H
#define DAP_PROTOCOL_H

#include "config.h"
#include "DAP_structs.h"
#include "DAP_hw_interface.h"

#include <cstdint>
#include <array>
#include <functional>

class DAP_Protocol {
public:

    explicit DAP_Protocol(DAP_hw_interface & hw);

    bool check_for_transfer_abort(const uint8_t * request);

    struct ret_t {
        uint16_t request_consumed;
        uint16_t response_size;
    };

    ret_t process_request(usb_buf_t & request,
                          usb_buf_t & response);

    inline void set_serial(const char * s) {
        _serial = s;
    }

    static std::function<void(bool)> connected_cb;
    static std::function<void(bool)> running_cb;

private:

    // Our access to the hardware pins
    DAP_hw_interface & _hw;

    // Process a single DAP request
    void process_request();

    // Process multiple DAP requests. It is assumed
    // that _request_index is pointing to the number
    // of requests!
    void process_requests();

    // Command implementations
    //////////////////////////
    void cmd_info();
    void cmd_host_status();
    void cmd_connect();
    void cmd_disconnect();

    void cmd_transfer_configure();
    void cmd_transfer();
    void cmd_transfer_block();
    void cmd_transfer_abort();

    void cmd_write_abort();
    void cmd_delay();
    void cmd_reset_target();

    void cmd_swj_pins();
    void cmd_swj_clock();
    void cmd_swj_sequence();

    void cmd_swd_configure();
    void cmd_swd_sequence();

    void cmd_jtag_configure();
    void cmd_jtag_sequence();
    void cmd_jtag_idcode();

    void cmd_swo_transport();
    void cmd_swo_mode();
    void cmd_swo_baudrate();
    void cmd_swo_control();
    void cmd_swo_status();
    void cmd_swo_ext_status();
    void cmd_swo_data();

    void cmd_uart_transport();
    void cmd_uart_configure();
    void cmd_uart_transfer();
    void cmd_uart_control();
    void cmd_uart_status();

    // Helper methods
    uint8_t  request_get_byte();
    uint16_t request_get_short();
    uint32_t request_get_word();

    void response_add_byte   (uint8_t );
    void response_add_short  (uint16_t);
    void response_add_word   (uint32_t);
    void response_add_string (const char *);
    void response_set_byte_at(uint16_t idx, uint8_t);
    void response_set_short_at(uint16_t idx, uint16_t);

    void queue_add_byte(uint8_t b);
    uint8_t queue_get_byte();

    bool select_device(int index);

    transfer_response_t transfer_word(transfer_request_t req, uint32_t & data);

    bool needs_posted_read(transfer_request_t request);

    uint32_t parity(uint32_t value);

    transfer_response_t jtag_operation(transfer_request_t req, uint32_t & data);
    void                jtag_write_ir(int ir);
    transfer_response_t swd_operation (transfer_request_t req, uint32_t & data);

    /////////////
    // Attributes
    /////////////
    std::array<void (DAP_Protocol::*)(), 36> _handlers;
    const char * _serial {nullptr};

    const uint8_t * _request {nullptr};
    uint16_t    _request_size {0};
    uint16_t    _request_index {0};

    uint8_t *   _response {nullptr};
    uint16_t    _response_size_max {0};
    uint16_t    _response_index {0};

    std::array<uint8_t, 1024> _queue;
    size_t      _queue_index {0};

    port_t      _port {PORT_DEFAULT};
    volatile bool _transfer_abort {false};

    uint8_t     _idle_cycles {0};
    uint16_t    _wait_retry  {100};
    uint16_t    _match_retry {100};

    uint32_t    _match_mask {0};

    uint8_t     _swd_turnaround {1};
    bool        _swd_data_phase {false};

    uint8_t     _jtag_dev_count {0};
    uint8_t     _jtag_dev_index {0};
    uint8_t     _jtag_ir_length[JTAG_DEV_COUNT];
    uint8_t     _jtag_ir_before[JTAG_DEV_COUNT];
    uint8_t     _jtag_ir_after [JTAG_DEV_COUNT];
    uint8_t     _jtag_ir {0};
};

#endif // DAP_PROTOCOL_H
