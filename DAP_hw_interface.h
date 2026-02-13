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
// This file defines a hardware interface to CMSIS DAP
// SWD and JTAG functionality. The concrete implementation
// has to provide some generic (timing) methods as well as
// basic read/write access to the SWD/JTAG Pins and simple
// methods to read/write a number of bits or generating
// clock cycles.
//
#ifndef DAP_HW_INTERFACE_H
#define DAP_HW_INTERFACE_H

#include <cstdint>

class DAP_hw_interface {
public:

    ////////////////////////////////
    // General configuration methods
    ////////////////////////////////

    // Set HW frequency in Hz, which drives the SWCLK/TCK Pin.
    virtual void frequency_set(uint32_t f) = 0;

    // Delay for a number of microseconds, which is
    // used at some places in the CMSIS DAP protocol.
    virtual void delay_us(uint32_t us) = 0;

    // CMSIS DAP can use an optional test domain timer.
    // The following methods return the support status,
    // the frequency and the current value of the test
    // domain timer.
    virtual bool     test_domain_timer_support() = 0;
    virtual uint32_t test_domain_timer_frequency() = 0;
    virtual uint32_t test_domain_timer_get() = 0;

    ////////////////////////////
    // Pin configuration methods
    ////////////////////////////

    // Set up all needed HW Pins for JTAG operation
    // (TCK, TMS, TDI, TDO and optionally nTRST and nRESET)
    virtual void connect_jtag_pins() = 0;

    // Set up all needed HW Pins for SWD operation
    // (SWCLK, SWDIO and optionally nRESET)
    virtual void connect_swd_pins() = 0;

    // De-configure all SWD/JTAG Pins and put them
    // into a high-Z state
    virtual void disconnect() = 0;

    ////////////////////////////////
    // SWD / JTAG read write methods
    ////////////////////////////////

    // Method for toggling the SWCLK/TCK line for a
    // certain amount of cycles. Used by SWD and JTAG.
    // Starting level of the SWCLK/TCK line is LOW,
    // so ony cycle is a -> HIGH -> LOW transition
    // (using the configured frequency).
    virtual void swclk_tck_cycle(uint16_t cycles) = 0;

    // Methods for reading up to 32 bits via the
    // SWD interface. Starting level of SWCLK is LOW.
    // SWDIO data is changed by the target on rising clock
    // edges. The host can read either after the falling
    // clock edge, or immediately before the next rising
    // clock edge.
    // NOTE: The first bit is driven by the target right
    // at the beginning of this method (Data Phase Shift)!
    // size is the number of bits to read.
    virtual uint32_t swd_read(uint8_t size) = 0;

    // Method for writing up to 32 bits via the SWD
    // interface. The first bit has to be prepared BEFORE
    // the first rising clock edge (when SWDIO is sampled
    // by the target). size is the number of bits to write.
    virtual void swd_write(uint32_t value, uint8_t size) = 0;

    // Methods for reading/writing up to 32 bits via the
    // JTAG interface. Starting level of TCK is HIGH.
    // TDI data is changed on falling clock edges (writing
    // to target), and TDO is sampled on rising edges (reading
    // from target). The size parameter is the number of bits to
    // read/write. The jtag_write method returns the shifted
    // input value (the 'remaining' bits after sending 'size'
    // bits).
    virtual uint32_t jtag_read(uint8_t size) = 0;
    virtual uint32_t jtag_write(uint32_t value, uint8_t size) = 0;
    virtual uint32_t jtag_read_write(uint32_t value, uint8_t size) = 0;

    ///////////////////////////////////////////
    // Direct SWD/JTAG Pin access (bit banging)
    ///////////////////////////////////////////

    // SWCLK / TCK Pin
    virtual void swclk_tck_set(bool v) = 0;
    virtual bool swclk_tck_get() = 0;

    // SWDIO / TMS Pin
    virtual void swdio_tms_set(bool v) = 0;
    virtual bool swdio_tms_get() = 0;
    virtual void swdio_tms_mode_input() = 0;
    virtual void swdio_tms_mode_output() = 0;

    // TDI Pin
    virtual void tdi_set(bool v) = 0;
    virtual bool tdi_get() = 0;

    // TDO Pin
    virtual bool tdo_get() = 0;

    // Optional /TRST Pin
    virtual void trst_set(bool v) = 0;
    virtual bool trst_get() = 0;

    // Optional /RESET Pin
    virtual bool reset_set(bool v) = 0;
    virtual bool reset_get() = 0;
};

#endif // DAP_HW_INTERFACE_H
