//////////////////////////////////////////////////////
//   This file is part of openDAP++, a C++ based
//   implementation of the CMSIS DAP protocol.
//   https://github.com/Terstegge/openDAPpp.git
//
//   (c) A. Terstegge (Andreas.Terstegge@gmail.com)
//////////////////////////////////////////////////////
//
// This file defines the hardware interface to CMSIS DAP
// SWD and JTAG functionality. Unlike other libraries,
// openDAP++ does not use a purely GPIO-based bit-banging
// interface for the various signals (SWDIO, SWCLK, TDI,
// TDO, TCK, TMS RESET), but expects the implementation
// of simple methods to read/write a number of bits or
// to generate a sequence of clock cycles.
// These methods make it easier to port the library to
// other HW layers which use SPI peripherals or embedded
// coprocessors (like e.g. the PIO units on a RP2040).
// Still, methods to read/write the several signals also
// have to be provided because of the DAP_SWJ_PINS command.
// A delay method also has to be implemented (this is used
// at some places in the CMSIS DAP protocol).
// Optionally a test domain timer can be implemented, which
// is used to time-stamp the SWD/JTAG replies.
//
#ifndef DAP_HW_INTERFACE_H
#define DAP_HW_INTERFACE_H

#include <cstdint>

class DAP_hw_interface {
public:

    ///////////////////////////////
    // Timing configuration methods
    ///////////////////////////////

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

    ///////////////////////////
    // Common operation methods
    ///////////////////////////

    // Method for toggling the SWCLK/TCK line for a
    // certain amount of cycles. Used by SWD and JTAG.
    // Starting level of the SWCLK/TCK line is LOW,
    // so one cycle is a -> HIGH -> LOW transition
    // (using the configured frequency).
    virtual void clock_cycle(uint16_t cycles) = 0;

    /////////////////////////
    // SWD read write methods
    /////////////////////////

    // Methods for reading up to 32 bits via the
    // SWD interface. Starting level of SWCLK is LOW.
    // This method has to make sure that SWDIO is set
    // to input mode at the beginning!
    // SWDIO data is changed at the next rising clock
    // edge, so the first bit has to be read BEFORE the
    // first clock cycle!
    // 'size' is the number of bits to be read (1..32);
    virtual uint32_t swd_read(uint8_t size) = 0;

    // Method for writing up to 32 bits via the
    // SWD interface. Starting level of SWCLK is LOW.
    // This method has to make sure that SWDIO is set
    // to output mode at the beginning!
    // SWDIO data is sampled at the next rising clock
    // edge, so the first bit has to be prepared BEFORE
    // the first clock cycle.
    // 'size' is the number of bits to be written (1..32).
    virtual void swd_write(uint32_t value, uint8_t size) = 0;

    // Set the mode (input/output) of the SWDIO line. This
    // method might be used by swd_read and swd_write.
    virtual void swd_swdio_enable_output(bool b);

    //////////////////////////
    // JTAG read write methods
    //////////////////////////

    // Methods for reading up to 32 bits via the
    // JTAG interface. Starting level of TCK is LOW.
    // TDO data is changed at the next rising clock
    // edge, so the first bit has to be read BEFORE the
    // first clock cycle!
    // 'size' is the number of bits to be read (1..32);
    virtual uint32_t jtag_read(uint8_t size) = 0;

    // Method for writing up to 32 bits via the
    // JTAG interface. Starting level of TCK is LOW.
    // TDI data is sampled at the next rising clock
    // edge, so the first bit has to be prepared BEFORE
    // the first clock cycle.
    // 'size' is the number of bits to be written (1..32).
    virtual uint32_t jtag_write(uint32_t value, uint8_t size) = 0;

    // Combination of jtag_read and jtag_write (see above).
    virtual uint32_t jtag_read_write(uint32_t value, uint8_t size) = 0;

    /////////////////////////////
    // Direct SWD/JTAG Pin access
    /////////////////////////////

    // SWDIO / TMS Pin
    virtual void swdio_tms_set(bool v) = 0;
    virtual bool swdio_tms_get() = 0;

    // SWCLK / TCK Pin
    virtual void swclk_tck_set(bool v) = 0;
    virtual bool swclk_tck_get() = 0;

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
