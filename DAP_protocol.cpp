
//
#include <cstring>
#include <cassert>
#include "DAP_protocol.h"
#include "DAP_log.h"

using enum DAP_log::log_level;

std::function<void(bool)> DAP_Protocol::connected_cb;
std::function<void(bool)> DAP_Protocol::running_cb;

DAP_Protocol::DAP_Protocol(DAP_hw_interface & hw) : _hw(hw) {
    DAP_LOG(LOG_DEBUG, "DAP_Protocol()");

    _handlers[CMD_INFO]               = &DAP_Protocol::cmd_info;
    _handlers[CMD_HOST_STATUS]        = &DAP_Protocol::cmd_host_status;
    _handlers[CMD_CONNECT]            = &DAP_Protocol::cmd_connect;
    _handlers[CMD_DISCONNECT]         = &DAP_Protocol::cmd_disconnect;

    _handlers[CMD_TRANSFER_CONFIGURE] = &DAP_Protocol::cmd_transfer_configure;
    _handlers[CMD_TRANSFER]           = &DAP_Protocol::cmd_transfer;
    _handlers[CMD_TRANSFER_BLOCK]     = &DAP_Protocol::cmd_transfer_block;
    _handlers[CMD_TRANSFER_ABORT]     = &DAP_Protocol::cmd_transfer_abort;

    _handlers[CMD_WRITE_ABORT]        = &DAP_Protocol::cmd_write_abort;
    _handlers[CMD_DELAY]              = &DAP_Protocol::cmd_delay;
    _handlers[CMD_RESET_TARGET]       = &DAP_Protocol::cmd_reset_target;

    _handlers[CMD_SWJ_PINS]           = &DAP_Protocol::cmd_swj_pins;
    _handlers[CMD_SWJ_CLOCK]          = &DAP_Protocol::cmd_swj_clock;
    _handlers[CMD_SWJ_SEQUENCE]       = &DAP_Protocol::cmd_swj_sequence;

    _handlers[CMD_SWD_CONFIGURE]      = &DAP_Protocol::cmd_swd_configure;
    _handlers[CMD_SWD_SEQUENCE]       = &DAP_Protocol::cmd_swd_sequence;

    _handlers[CMD_JTAG_SEQUENCE]      = &DAP_Protocol::cmd_jtag_sequence;
    _handlers[CMD_JTAG_CONFIGURE]     = &DAP_Protocol::cmd_jtag_configure;
    _handlers[CMD_JTAG_IDCODE]        = &DAP_Protocol::cmd_jtag_idcode;

    _handlers[CMD_SWO_TRANSPORT]      = &DAP_Protocol::cmd_swo_transport;
    _handlers[CMD_SWO_MODE]           = &DAP_Protocol::cmd_swo_mode;
    _handlers[CMD_SWO_BAUDRATE]       = &DAP_Protocol::cmd_swo_baudrate;
    _handlers[CMD_SWO_CONTROL]        = &DAP_Protocol::cmd_swo_control;
    _handlers[CMD_SWO_STATUS]         = &DAP_Protocol::cmd_swo_status;
    _handlers[CMD_SWO_EXT_STATUS]     = &DAP_Protocol::cmd_swo_ext_status;
    _handlers[CMD_SWO_DATA]           = &DAP_Protocol::cmd_swo_data;

    _handlers[CMD_UART_TRANSPORT]     = &DAP_Protocol::cmd_uart_transport;
    _handlers[CMD_UART_CONFIGURE]     = &DAP_Protocol::cmd_uart_configure;
    _handlers[CMD_UART_TRANSFER]      = &DAP_Protocol::cmd_uart_transfer;
    _handlers[CMD_UART_CONTROL]       = &DAP_Protocol::cmd_uart_control;
    _handlers[CMD_UART_STATUS]        = &DAP_Protocol::cmd_uart_status;

    _port = PORT_DEFAULT;
    _hw.frequency_set(DAP_DEFAULT_CLOCK);
}

bool DAP_Protocol::check_for_transfer_abort(const uint8_t * request) {
    if (*request == CMD_TRANSFER_ABORT) {
        _transfer_abort = true;
        return true;
    }
    return false;
}

DAP_Protocol::ret_t DAP_Protocol::process_request(usb_buf_t & request,
                                                  usb_buf_t & response) {
    DAP_LOG(LOG_DEBUG, "process_request()");

    // Initialize buffers
    _request        = request.data;
    _request_size   = request.len;
    _request_index  = 0;

    _response       = response.data;
    _response_size_max  = sizeof(response.data);
    _response_index = 0;

    // Rest abort flag
    _transfer_abort = false;

    // Look at first byte in request without consuming it
    uint8_t cmd = _request[_request_index];

    // Check for queued requests which we have to process NOW!
    if ((cmd != CMD_QUEUE_COMMANDS) && (_queue_index != 0)) {
        _request = _queue.data();
        while (_request_index < _queue_index) {
            uint8_t queue = queue_get_byte();
            assert(queue == CMD_QUEUE_COMMANDS);
            response_add_byte(CMD_EXECUTE_COMMANDS);
            process_requests();
        }
        // Reset queue and continue with current request..
        _queue_index = 0;
        _request = request.data;
        _request_index = 0;
        cmd = _request[_request_index];
    }

    if (cmd < _handlers.size()) {
        process_request();
    } else if (cmd == CMD_EXECUTE_COMMANDS) {
        response_add_byte(request_get_byte());
        process_requests();
    } else if (cmd == CMD_QUEUE_COMMANDS) {
        for(size_t i=0; i < request.len; ++i) {
            queue_add_byte(request_get_byte());
        }
        return {
            .request_consumed = request.len,
            .response_size    = 0 // NO response!
        };
    } else if (cmd >= CMD_VENDOR_0 && cmd <= CMD_VENDOR_31) {
        response_add_byte(request_get_byte());
        response_add_byte(STATUS_ERROR);
    }
    else {
        response_add_byte(CMD_INVALID);
    }

    response.len = _response_index;

    return {
        .request_consumed = _request_index,
        .response_size    = _response_index
    };
}

void DAP_Protocol::process_request() {
    uint8_t cmd = request_get_byte();
    response_add_byte(cmd);
    assert(cmd < _handlers.size());
    if (_handlers[cmd]) {
        (this->*(_handlers[cmd]))();
    }
}

void DAP_Protocol::process_requests() {
    uint8_t count = request_get_byte();
    response_add_byte(count);
    for (int i = 0; i < count; ++i) {
        process_request();
    }
}

/////////////////////////////
// DAP command implementation
/////////////////////////////

void DAP_Protocol::cmd_info() {
    DAP_LOG(LOG_INFO, "cmd_info(index: 0x%x)", _request[_request_index]);

    int index = request_get_byte();
    switch(index) {
        case INFO_VENDOR_NAME: {
            response_add_string(DAP_VENDOR_NAME);
            break;
        }
        case INFO_PRODUCT_NAME: {
            response_add_string(DAP_PRODUCT_NAME);
            break;
        }
        case INFO_SERIAL_NUMBER: {
            response_add_string(_serial);
            break;
        }
        case INFO_CMSIS_DAP_VERSION: {
            response_add_string(DAP_PROTOCOL_VERSION);
            break;
        }
        case INFO_DEVICE_VENDOR: {
            response_add_string(DAP_DEVICE_VENDOR);
            break;
        }
        case INFO_DEVICE_NAME: {
            response_add_string(DAP_DEVICE_NAME);
            break;
        }
        case INFO_BOARD_VENDOR: {
            response_add_string(DAP_BOARD_VENDOR);
            break;
        }
        case INFO_BOARD_NAME: {
            response_add_string(DAP_BOARD_NAME);
            break;
        }
        case INFO_FIRMWARE_VERSION: {
            response_add_string(DAP_FIRMWARE_VERSION);
            break;
        }
        case INFO_CAPABILITIES: {
            capabilities_t cap = {
                .swd_support         = DAP_CAP_SWD_SUPPORT,
                .jtag_support        = DAP_CAP_JTAG_SUPPORT,
                .swo_uart            = DAP_CAP_SWO_UART,
                .swo_manchester      = DAP_CAP_SWO_MANCHESTER,
                .atomic_commands     = true, // implemented!
                .test_domain_timer   = _hw.test_domain_timer_support(),
                .swo_streaming_trace = DAP_CAP_SWO_STREAMING_TRACE,
                .uart_com_port       = DAP_CAP_UART_COM_PORT,
                .usb_com_port        = DAP_CAP_USB_COM_PORT
            };
            response_add_byte(sizeof(capabilities_t));
            response_add_byte(cap.info0);
            response_add_byte(cap.info1);
            break;
        }
        case INFO_TEST_DOMAIN_TIMER: {
            response_add_byte(sizeof(uint32_t));
            response_add_word(_hw.test_domain_timer_frequency());
            break;
        }
        case INFO_UART_RX_SIZE: {
            response_add_byte(sizeof(uint32_t));
            response_add_word(DAP_UART_RX_SIZE);
            break;
        }
        case INFO_UART_TX_SIZE: {
            response_add_byte(sizeof(uint32_t));
            response_add_word(DAP_UART_TX_SIZE);
            break;
        }
        case INFO_SWO_BUFFER_SIZE: {
            response_add_byte(sizeof(uint32_t));
            response_add_word(DAP_SWO_BUFFER_SIZE);
            break;
        }
        case INFO_MAX_PACKET_COUNT: {
            response_add_byte(sizeof(uint8_t));
            response_add_byte(DAP_MAX_PACKET_COUNT);
            break;
        }
        case INFO_MAX_PACKET_SIZE: {
            response_add_byte(sizeof(uint16_t));
            response_add_short(DAP_MAX_PACKET_SIZE);
            break;
        }
        default: {
            // Return zero-length packet
            response_add_byte(0);
        }
    }
}

void DAP_Protocol::cmd_host_status() {
    uint8_t type  = request_get_byte();
    uint8_t state = request_get_byte();
    DAP_LOG(LOG_INFO, "cmd_host_status(type:%d, state:%d)", type, state);

    switch(type) {
        case 0: {
            if (connected_cb) connected_cb(state);
            break;
        }
        case 1: {
            if (running_cb) running_cb(state);
            break;
        }
        default: break;
    }
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_connect() {
    DAP_LOG(LOG_INFO, "cmd_connect()");

    int port_req = request_get_byte();
    switch(port_req) {
        case PORT_DEFAULT:
            _port = DAP_DEFAULT_PORT; break;
        case PORT_SWD:
            _port = PORT_SWD; break;
        case PORT_JTAG:
            _port = PORT_JTAG; break;
        default: break;
    }
    switch(_port) {
        case PORT_SWD:
            _hw.connect_swd_pins(); break;
        case PORT_JTAG:
            _hw.connect_jtag_pins(); break;
        default: break;
    }
    response_add_byte(_port);
}

void DAP_Protocol::cmd_disconnect() {
    DAP_LOG(LOG_INFO, "cmd_disconnect()");

    _hw.disconnect();
    _port = PORT_DEFAULT;
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_transfer_configure() {
    DAP_LOG(LOG_INFO, "cmd_transfer_configure()");

    _idle_cycles = request_get_byte();
    _wait_retry  = request_get_short();
    _match_retry = request_get_short();
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_transfer() {
    DAP_LOG(LOG_DEBUG, "cmd_transfer()");

    // Prepare the response. These two bytes
    // will have to be modified before we return...
    transfer_response_t response;
    response.ack = ack_t::invalid;
    response_add_byte(0); // Transfer count
    response_add_byte(response.value);

    // Get device and configure it. This is
    // only relevant for a JTAG interface.
    if (!select_device( request_get_byte() )) {
        return;
    }

    uint8_t request_count  = request_get_byte();
    uint8_t response_count = 0;

    bool posted_read = false, verify_write = false;
    uint32_t data = 0;

    // Loop over all transfer requests
    for(response_count=0; response_count < request_count; ++response_count) {
        // Check for abort
        if (_transfer_abort) break;

        // Get the next request
        transfer_request_t request;
        request.value = request_get_byte();
        verify_write = false;

        // Check if we have a delayed read
        if (posted_read) {
            if (needs_posted_read(request)) {
                // The next request is also delayed...
                if (request.time_stamp) {
                    response_add_word(_hw.test_domain_timer_get());
                }
                response = transfer_word(request, data);
            } else {
                // Next request is not delayed. Process
                // the last one and continue with next..
                response = transfer_word(DP_READ_RDBUFF, data);
                posted_read = false;
            }
            if (response.ack != ack_t::okay) {
                break;
            }
            response_add_word(data);
            // Start over if the next request also has a delayed read
            if (posted_read) continue;
        }

        if (request.read) {
            // Match value
            if (request.match_value) {
                uint32_t match_value = request_get_word();

                // If the register to match is a delayed one,
                // perform one first read...
                if (needs_posted_read(request)) {
                    response = transfer_word(request, data);
                    if (response.ack != ack_t::okay) {
                        break;
                    }
                }

                bool value_mismatch = true;
                for (uint16_t i=0; i < _match_retry; ++i) {
                    if (_transfer_abort) break;
                    response = transfer_word(request, data);
                    if ((data & _match_mask) == match_value) {
                        value_mismatch = false;
                        break;
                    }
                }
                response.value_mismatch = value_mismatch;
                if (response.ack != ack_t::okay) {
                    break;
                }
            } else {
                posted_read = needs_posted_read(request);
                response = transfer_word(request, data);
                if (response.ack != ack_t::okay) {
                    break;
                }
                if (request.time_stamp) {
                    response_add_word(_hw.test_domain_timer_get());
                }
                if (!posted_read) {
                    response_add_word(data);
                }
            }
        } else {
            data = request_get_word();

            if (request.match_mask) {
                _match_mask = data;
                response.ack = ack_t::okay;
            } else {
                response = transfer_word(request, data);
                if (request.time_stamp) {
                    response_add_word(_hw.test_domain_timer_get());
                }
                if (response.ack != ack_t::okay) {
                    break;
                }
                verify_write = true;
            }
        }
    }

    if (response.ack == ack_t::okay) {
        // Did we have a delayed read or verify write
        // in the last request?
        if (posted_read || verify_write) {
            response = transfer_word(DP_READ_RDBUFF, data);
            if (posted_read) {
                response_add_word(data);
            }
        }
    }

    // Finally update the response count and status
    response_set_byte_at(1, response_count);
    response_set_byte_at(2, response.value);
}

void DAP_Protocol::cmd_transfer_block() {
    DAP_LOG(LOG_DEBUG, "cmd_transfer_block()");

    // Prepare the response. These three bytes
    // will have to be modified before we return...
    transfer_response_t response;
    response.ack = ack_t::invalid;
    response_add_short(0); // Transfer count
    response_add_byte(response.value);

    // Get device and configure it. This is
    // only relevant for a JTAG interface.
    if (!select_device( request_get_byte() )) {
        return;
    }

    uint16_t request_count  = request_get_short();
    uint16_t response_count = 0;

    // Check for zero transfers
    if (request_count == 0) return;

    // Get the request information
    transfer_request_t request;
    request.value = request_get_byte();

    uint32_t data;
    if (request.read) {

        // If delayed read, read one word extra
        bool needs_posted = needs_posted_read(request);
        if (needs_posted) {
            response = transfer_word(request, data);
            if (response.ack != ack_t::okay) {
                return;
            }
        }
        for (response_count = 0; response_count < request_count; response_count++) {
            // Check for the last read with delay
            if (needs_posted && (response_count == (request_count-1))) {
                request = DP_READ_RDBUFF;
            }
            response = transfer_word(request, data);
            if (response.ack != ack_t::okay) {
                break;
            }
            response_add_word(data);
        }
    } else {
        // Write 'request_count' words
        for (response_count = 0; response_count < request_count; response_count++) {
            data = request_get_word();
            response = transfer_word(request, data);
            if (response.ack != ack_t::okay) break;
        }
        // Make the final verify read
        if (response.ack == ack_t::okay) {
            response = transfer_word(DP_READ_RDBUFF, data);
        }
    }

    // Finally update the response count and status
    response_set_short_at(1, response_count);
    response_set_byte_at (3, response.value);
}

void DAP_Protocol::cmd_transfer_abort() {
    DAP_LOG(LOG_INFO, "cmd_transfer_abort()");
    // This request is handled outside the normal queue.
    // We should never get here.
    response_add_byte(STATUS_OK);
}



void DAP_Protocol::cmd_write_abort() {
    DAP_LOG(LOG_INFO, "cmd_write_abort()");

    uint32_t data;
    // Get device and configure it. This is
    // only relevant for a JTAG interface.
    if (!select_device(request_get_byte())) {
        response_add_byte(STATUS_ERROR);
        return;
    }
    data = request_get_word();

    if (_port == PORT_SWD) {
        swd_operation(DP_WRITE_ABORT, data);
    } else if (_port == PORT_JTAG) {
        //req.transfer_abort_ = true;
        jtag_operation(DP_WRITE_ABORT, data);
    }
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_delay() {
    uint16_t delay = request_get_short();
    DAP_LOG(LOG_INFO, "cmd_delay(%dus)", delay);
    _hw.delay_us(delay);
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_reset_target() {
    DAP_LOG(LOG_INFO, "cmd_reset_target()");
    response_add_byte(STATUS_OK);
    // bool res = _hw.nRESET_set();
    // DAP_CONFIG_RESET_TARGET_FN();
    response_add_byte(0);
}






void DAP_Protocol::cmd_swj_pins() {
    DAP_LOG(LOG_INFO, "cmd_swj_pins()");

    pin_mappings_t value;
    value.value = request_get_byte();
    pin_mappings_t select;
    select.value  = request_get_byte();
    uint32_t wait = request_get_word();

    if (select.SWCLK_TCK)
        _hw.swclk_tck_set(value.SWCLK_TCK);
    if (select.SWDIO_TMS)
        _hw.swdio_tms_set(value.SWDIO_TMS);
    if (select.TDI)
        _hw.tdi_set(value.TDI);
    if (select.nTRST)
        _hw.trst_set(value.nTRST);
    if (select.nRESET)
        _hw.reset_set(value.nRESET);

    _hw.delay_us(wait);

    pin_mappings_t result;
    result.SWCLK_TCK = _hw.swclk_tck_get();
    result.SWDIO_TMS = _hw.swdio_tms_get();
    result.TDI       = _hw.tdi_get();
    result.TDO       = _hw.tdo_get();
    result.nTRST     = _hw.trst_get();
    result.nRESET    = _hw.reset_get();

    response_add_byte(result.value);
}

void DAP_Protocol::cmd_swj_clock() {
    uint32_t freq = request_get_word();
    DAP_LOG(LOG_DEBUG, "cmd_swj_clock(%d)", freq);
    _hw.frequency_set(freq);
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_swj_sequence() {
    uint16_t size = request_get_byte();
    DAP_LOG(LOG_DEBUG, "cmd_swj_sequence(%d)", size);

    if (size == 0) size = 256;

    while (size) {
        uint8_t sz = (size > 8) ? 8 : size;
        swd_write(request_get_byte(), sz);
        size -= sz;
    }
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_swd_configure() {
    DAP_LOG(LOG_INFO, "cmd_swd_configure()");

    swd_config_t config;
    config.value = request_get_byte();

    _swd_turnaround = config.turnaround + 1;
    _swd_data_phase = config.data_phase;

    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_swd_sequence() {

    if (_port != PORT_SWD) {
        response_add_byte(STATUS_ERROR);
        return;
    }

    response_add_byte(STATUS_OK);

    uint8_t req_count = request_get_byte();

    for (uint8_t i= 0; i < req_count; i++) {
        swd_sequence_info_t info;
        info.value = request_get_byte();

        uint8_t cycles = info.cycles;
        if (cycles == 0) cycles = 64;

        if (info.read) {
            _hw.swdio_tms_mode_input();
            while (cycles) {
                int c = (cycles > 8) ? 8 : cycles;
                uint8_t value = swd_read(c);
                response_add_byte(value);
                cycles -= c;
            }
        } else {
            _hw.swdio_tms_mode_output();
            while (cycles) {
                int c = (cycles > 8) ? 8 : cycles;
                swd_write(request_get_byte(), c);
                cycles -= c;
            }
        }
    }
    _hw.swdio_tms_mode_output();
}

void DAP_Protocol::cmd_jtag_configure() {
    uint8_t count = request_get_byte();
    int bits = 0;

    if (count > JTAG_DEV_COUNT) {
        response_add_byte(STATUS_ERROR);
        return;
    }

    _jtag_dev_count = count;
    _jtag_dev_index = 0;

    for (int i=0; i < _jtag_dev_count; i++) {
        _jtag_ir_length[i] = request_get_byte();
        _jtag_ir_before[i] = bits;
        bits += _jtag_ir_length[i];
    }
    for (int i=0; i < _jtag_dev_count; i++) {
        bits -= _jtag_ir_length[i];
        _jtag_ir_after[i] = bits;
    }
    response_add_byte(STATUS_OK);
}

void DAP_Protocol::cmd_jtag_sequence() {

    if (_port != PORT_JTAG){
        response_add_byte(STATUS_ERROR);
        return;
    }

    response_add_byte(STATUS_OK);

    uint8_t req_count = request_get_byte();

    for (int i = 0; i < req_count; i++) {
        jtag_sequence_info_t info;
        info.value = request_get_byte();

        uint8_t cycles = info.cycles;
        if (cycles == 0) cycles = 64;

        _hw.swdio_tms_set(info.tms_value);

        while (cycles) {
            int c = (cycles > 8) ? 8 : cycles;
            if (info.tdo_capture) {
                uint8_t value = jtag_read_write(request_get_byte(), c);
                response_add_byte(value);
            } else {
                jtag_write(request_get_byte(), c);
            }
            cycles -= c;
        }
    }
}

void DAP_Protocol::cmd_jtag_idcode() {
    uint32_t data;

    if (_port != PORT_JTAG ||
        !select_device(request_get_byte())) {
        response_add_byte(STATUS_ERROR);
        return;
    }

    jtag_write_ir(JTAG_IDCODE);

    _hw.swdio_tms_set(true);
    swj_cycle(1); // -> Select-DR-Scan
    _hw.swdio_tms_set(false);
    swj_cycle(2 + _jtag_dev_index); // -> Shift-DR

    data = jtag_read(31);

    _hw.swdio_tms_set(true);
    data |= (jtag_read(1) << 31); // -> Exit1-DR

    swj_cycle(1); // -> Update-DR
    _hw.swdio_tms_set(false);
    swj_cycle(1); // -> Idle
    _hw.tdi_set(true);

    response_add_byte(STATUS_OK);
    response_add_word(data);
}

void DAP_Protocol::cmd_swo_transport() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_swo_mode() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_swo_baudrate() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_swo_control() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_swo_status() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_swo_ext_status() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_swo_data() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_uart_transport() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_uart_configure() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_uart_transfer() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_uart_control() {
    response_add_byte(STATUS_ERROR);
}

void DAP_Protocol::cmd_uart_status() {
    response_add_byte(STATUS_ERROR);
}

/////////////////
// Helper Methods
/////////////////

uint8_t DAP_Protocol::request_get_byte() {
    return _request[_request_index++];
    assert(_request_index <= _request_size);
}

uint16_t DAP_Protocol::request_get_short() {
     return (uint16_t)request_get_byte() |
           ((uint16_t)request_get_byte() << 8);
}

uint32_t DAP_Protocol::request_get_word() {
    return (uint32_t)request_get_byte() |
          ((uint32_t)request_get_byte() << 8) |
          ((uint32_t)request_get_byte() << 16) |
          ((uint32_t)request_get_byte() << 24);
}

void DAP_Protocol::response_add_byte(uint8_t value) {
    _response[_response_index++] = value;
    assert(_response_index <= _response_size_max);
}

void DAP_Protocol::response_add_short(uint16_t value) {
    _response[_response_index++] = value;
    _response[_response_index++] = value >> 8;
    assert(_response_index <= _response_size_max);
}

void DAP_Protocol::response_add_word(uint32_t value) {
    _response[_response_index++] = value;
    _response[_response_index++] = value >> 8;
    _response[_response_index++] = value >> 16;
    _response[_response_index++] = value >> 24;
    assert(_response_index <= _response_size_max);
}

void DAP_Protocol::response_add_string(const char * s) {
    if (!s) {
        // No information -> zero length response
        response_add_byte(0);
    } else {
        response_add_byte(strlen(s)+1);
        size_t i=0;
        do {
            response_add_byte(s[i]);
        } while(s[i++]);
    }
}

void DAP_Protocol::response_set_byte_at(uint16_t idx, uint8_t value) {
    assert(idx < _response_size_max);
    _response[idx] = value;
}

void DAP_Protocol::response_set_short_at(uint16_t idx, uint16_t value) {
    assert(idx < (_response_size_max-1));
    _response[idx  ] = value;
    _response[idx+1] = value >> 8;
}


void DAP_Protocol::queue_add_byte(uint8_t value) {
    _queue[_queue_index++] = value;
    assert(_queue_index <= _queue.size());
}

uint8_t DAP_Protocol::queue_get_byte() {
    return _queue[_queue_index++];
    assert(_queue_index <= _queue.size());
}

bool DAP_Protocol::select_device(int index) {
    // Ignore for SWD
    if (_port == PORT_SWD)
        return true;

    if (_port == PORT_JTAG) {
        if (index >= _jtag_dev_count ||
            _jtag_ir_length[index] != JTAG_IR_LENGTH)
            return false;

        _jtag_dev_index = index;
        return true;
    }
    return false;
}

transfer_response_t DAP_Protocol::transfer_word(transfer_request_t request, uint32_t & data) {
    transfer_response_t res;
    res.value = 0;

    for (uint16_t i = 0; i < _wait_retry; i++) {
        if (_port == PORT_SWD) {
            res = swd_operation(request, data);
        } else if (_port == PORT_JTAG) {
            res = jtag_operation(request, data);
        }
        if (res.ack != ack_t::wait || _transfer_abort)
            break;
    }
    return res;
}

bool DAP_Protocol::needs_posted_read(transfer_request_t request) {
    if (!request.read)
        return false;

    if (_port == PORT_SWD)
        return (request.access_port);

    if (_port == PORT_JTAG)
        return true;

    return false;
}

uint32_t DAP_Protocol::parity(uint32_t value) {
    value ^= value >> 16;
    value ^= value >> 8;
    value ^= value >> 4;
    value &= 0x0f;
    return (0x6996 >> value) & 1;
}

//////////////////////////////////////////////////////
// Second-lowest layer: A single SWD or JTAG operation
//////////////////////////////////////////////////////

transfer_response_t DAP_Protocol::jtag_operation(transfer_request_t req, uint32_t & data) {
    transfer_response_t resp;
    int ir;

//    if (req._abort_)
//        ir = JTAG_ABORT;
//    else
        ir = req.access_port ? JTAG_APACC : JTAG_DPACC;

    if (ir != _jtag_ir) {
        _jtag_ir = ir;
        jtag_write_ir(ir);
    }

    _hw.swdio_tms_set(true);
    swj_cycle(1); // -> Select-DR-Scan
    _hw.swdio_tms_set(false);
    swj_cycle(2 + _jtag_dev_index); // -> Shift-DR

    resp.value = jtag_read_write(req.value >> 1, 3);

    if (resp.ack == ack_t::wait)
        resp.ack = ack_t::okay; // or FAULT
    else if (resp.ack == ack_t::wait)
        resp.ack = ack_t::wait;
    else
        resp.ack = ack_t::invalid;

    if (resp.ack == ack_t::okay) {
        int cnt = _jtag_dev_count - _jtag_dev_index - 1;
        uint32_t value;

        if (req.read) {
            if (cnt) {
                value = jtag_read(32);
                swj_cycle(cnt - 1);
                _hw.swdio_tms_set(true);
                swj_cycle(1); // -> Exit1-DR
            } else {
                value = jtag_read(31);
                _hw.swdio_tms_set(true);
                value |= (jtag_read(1) << 31); // -> Exit1-DR
            }

            data = value;

        } else {
            value = data;

            if (cnt) {
                jtag_write(value, 32);
                swj_cycle(cnt - 1);
                _hw.swdio_tms_set(true);
                swj_cycle(1); // -> Exit1-DR
            } else {
                value = jtag_write(value, 31);
                _hw.swdio_tms_set(true);
                jtag_write(value, 1); // -> Exit1-DR
            }
        }
    } else {// Not OK
        _hw.swdio_tms_set(true);
        swj_cycle(1); // -> Exit1-DR
    }

    swj_cycle(1); // -> Update-DR
    _hw.swdio_tms_set(false);
    swj_cycle(1); // -> Idle
    _hw.tdi_set(true);

    swj_cycle(_idle_cycles);

    return resp;
}

void DAP_Protocol::jtag_write_ir(int ir) {
    int len = _jtag_ir_length[_jtag_dev_index];

    _hw.swdio_tms_set(true);
    swj_cycle(2); // -> Select-IR-Scan
    _hw.swdio_tms_set(false);
    swj_cycle(2); // -> Shift-IR

    _hw.tdi_set(true);
    swj_cycle(_jtag_ir_before[_jtag_dev_index]);

    ir = jtag_write(ir, len - 1);

    if (_jtag_ir_after[_jtag_dev_index]) {
        jtag_write(ir, 1);

        _hw.tdi_set(true);
        swj_cycle(_jtag_ir_after[_jtag_dev_index] - 1);

        _hw.swdio_tms_set(true);
        swj_cycle(1); // -> Exit1-IR
    } else {
        _hw.swdio_tms_set(true);
        jtag_write(ir, 1); // -> Exit1-IR
    }

    swj_cycle(1); // -> Update-IR
    _hw.swdio_tms_set(false);
    swj_cycle(1); // -> Idle
    _hw.tdi_set(true);
}

transfer_response_t DAP_Protocol::swd_operation(transfer_request_t req, uint32_t & data) {
    // Write header
    swd_header_t header;
    // Most bits are the same, but shifted...
    header.value = req.value << 1;
    // Set remaining bits
    header.start  = 1;
    header.stop   = 0;
    header.park   = 1;
    header.parity = parity(header.value);
    swd_write(header.value, 8);

    // Turnaround
    _hw.swdio_tms_mode_input();
    swj_cycle(_swd_turnaround);

    // Read ACK
    transfer_response_t resp;
    resp.value = swd_read(3);

    uint32_t value;
    if (resp.ack == ack_t::okay) {
        if (header.read) {
            value = swd_read(32);

            if (parity(value) != swd_read(1)) {
                resp.protocol_error = 1;
                resp.ack = ack_t::invalid;
            }

            data = value;

            swj_cycle(_swd_turnaround);
            _hw.swdio_tms_mode_output();
        } else {
            swj_cycle(_swd_turnaround);
            _hw.swdio_tms_mode_output();

            swd_write(data, 32);
            swd_write(parity(data), 1);
        }

        _hw.swdio_tms_set(false);
        swj_cycle(_idle_cycles);

    } else if (resp.ack == ack_t::wait ||
               resp.ack == ack_t::fault) {
        if (_swd_data_phase && header.read)
            swj_cycle(32 + 1);

        swj_cycle(_swd_turnaround);

        _hw.swdio_tms_mode_output();

        if (_swd_data_phase && !header.read) {
            _hw.swdio_tms_set(false);
            swj_cycle(32 + 1);
        }
    } else {
        swj_cycle(_swd_turnaround + 32 + 1);
    }

    _hw.swdio_tms_set(true);

    return resp;
}



//////////////////////////
// Lowest-level HW methods
//////////////////////////

void DAP_Protocol::swj_cycle(uint16_t cycles) {
    if (_hw.swclk_tck_cycle_support()) {
        _hw.swclk_tck_cycle(cycles);
        return;
    }
    while (cycles--) {
        _hw.swclk_tck_set(false);
        _hw.delay_edge();
        _hw.swclk_tck_set(true);
        _hw.delay_edge();
    }
}

// Read 1..32 bits from TDO
uint32_t DAP_Protocol::jtag_read(uint8_t size) {
    if (_hw.jtag_read_write_support()) {
        return _hw.jtag_read(size);
    }
    uint32_t value = 0;
    uint32_t bit;
    for (uint8_t i = 0; i < size; i++) {
        _hw.swclk_tck_set(false);
        _hw.delay_edge();
        bit = _hw.tdo_get();
        _hw.swclk_tck_set(true);
        _hw.delay_edge();
        value |= (bit << i);
    }
    return value;
}

// Write 1..32 bits to TDI
uint32_t DAP_Protocol::jtag_write(uint32_t value, uint8_t size) {
    if (_hw.jtag_read_write_support()) {
        return _hw.jtag_write(value, size);
    }
    for (uint8_t i = 0; i < size; i++) {
        _hw.tdi_set(value & 1);
        _hw.swclk_tck_set(false);
        _hw.delay_edge();
        _hw.swclk_tck_set(true);
        _hw.delay_edge();
        value >>= 1;
    }
    return value;
}

// Read/Write 1..32 bits from TDO/to TDI
uint32_t DAP_Protocol::jtag_read_write(uint32_t value, uint8_t size) {
    if (_hw.jtag_read_write_support()) {
        return _hw.jtag_read_write(value, size);
    }
    uint32_t read_value = 0;
    uint32_t bit;
    for (uint8_t i = 0; i < size; i++) {
        _hw.tdi_set(value & 1);
        _hw.swclk_tck_set(false);
        _hw.delay_edge();
        bit = _hw.tdo_get();
        _hw.swclk_tck_set(true);
        _hw.delay_edge();
        value >>= 1;
        read_value |= (bit << i);
    }
    return read_value;
}

// Read 1..32 bits from SWDIO
uint32_t DAP_Protocol::swd_read(uint8_t size) {
    if (_hw.swd_read_write_support()) {
        return _hw.swd_read(size);
    }
    uint32_t value = 0;
    uint32_t bit;
    for (uint8_t i = 0; i < size; i++) {
        _hw.swclk_tck_set(false);
        _hw.delay_edge();
        bit = _hw.swdio_tms_get();
        _hw.swclk_tck_set(true);
        _hw.delay_edge();
        value |= (bit << i);
    }
    return value;
}

// Write 1..32 bits to SWDIO
void DAP_Protocol::swd_write(uint32_t value, uint8_t size) {
    if (_hw.swd_read_write_support()) {
        _hw.swd_write(value, size);
        return;
    }
    for (uint8_t i = 0; i < size; i++) {
        _hw.swdio_tms_set(value & 1);
        _hw.swclk_tck_set(false);
        _hw.delay_edge();
        _hw.swclk_tck_set(true);
        _hw.delay_edge();
        value >>= 1;
    }
}

