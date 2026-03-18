# openDAP++

This is a free and open implementation of the CMSIS-DAP debugger firmware
written in C++. The Free-DAP library (https://github.com/ataradov/free-dap.git)
served as a code basis.

Both SWD and JTAG protocols are supported. However JTAG was not well tested due
to lack of JTAG targets. I tested with openOCD and a MSP432 Lauchpad in JTAG
mode. But openOCD will not use many of the CMSIS JTAG methods, but instead uses
mostly DAP_JTAG_Sequence commands to have full control over the JTAG chain.
If you have any issues with it - let me know and I'll try to help.

## Building a debugger with openDAP++

To create a CMSIS-DAP compliant debugger, you have to:
 * Implement a class implementing the interface defined in DAP_hw_interface.h
   (see this file for details which methods have to be provided)
 * Implement a USB device with raw bulk endpoints for CMSIS-DAP v2
 * Adapt the configuration file config.h (see config_sample.h)
 * Create a DAP_protocol instance, providing the implementation of the
   DAP_hw_interface as a CTOR parameter
 * Call process_request() for every received DAP request and send back
   the response

## CMSIS-DAP version support

openDAP++ library itself is protocol agnostic and implementation of the
specific version of the CMSIS-DAP protocol (v1 or v2) is up to the
individual platforms.

