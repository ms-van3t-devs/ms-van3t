**Rawsock library**

Raw sockets under Linux... made easier! C library for using raw sockets to send packets, supporting Linux. 

Starting from version 0.3.4, compilation of the main library on Android should be fully functional (e.g. when compiling on a terminal emulator such as Termux, using clang).

Version 0.3.4, supporting IPv4 and UDP, but ready for the addition of new protocols, such as WSMP. With the addition of a custom latency measurement L7 protocol (**LaMP** - <b>La</b>tency <b>M</b>easurement <b>P</b>rotocol).

![](./docs/pics/LaMP_logo.png)

The full documentation can be found at: https://francescoraves483.github.io/Rawsock_lib

This library is composed by two modules:
- one main module, **rawsock.h**, which can be used to more easily manage raw sockets, prepare, check and encapsulate packets to be sent over them and automatically look for certain types of interfaces available on the system (e.g. automatically look for WLAN interfaces)
- one additional add-on module, **rawsock_lamp.h**,  enabling support to a custom latency measurement protocol (_LaMP_ - supported both in raw and non-raw sockets)

Two example programs for broadcast communications (**Example_send.c** and **Example_receive.c**) are included, showing a possible use of the library. All the functions inside rawsock.c are documented through multi-line comments and inside this documentation, generated thanks to Doxygen.

A function (wlanLookup()) to automatically look for available wireless (or loopback) interfaces is included too.

This library is for Linux only, at the moment. Tested with Linux kernel 4.14.63 and later.

**Cross-compiling for OpenWrt, on PC Engines APU1D boards:**

If you are using this library to create programs to be cross-compiled and included on embedded boards, running OpenWrt, you can refer to the following instructions as a base for a correct cross-compilation. These commands are actually related to PC Engines APU1D boards, which are x86_64 targets, and to the example programs. They may differ if you are trying to compile for other boards. The OpenWrt toolchain must be correctly set up on your PC, too.

	x86_64-openwrt-linux-musl-gcc -I ./Rawsock_lib/ -o Example_send -static Example_send.c Rawsock_lib/rawsock.h Rawsock_lib/rawsock.c Rawsock_lib/ipcsum_alth.h Rawsock_lib/ipcsum_alth.c Rawsock_lib/minirighi_udp_checksum.h Rawsock_lib/minirighi_udp_checksum.c
	x86_64-openwrt-linux-musl-gcc -I ./Rawsock_lib/ -o Example_receive -static Example_receive.c Rawsock_lib/rawsock.h Rawsock_lib/rawsock.c Rawsock_lib/ipcsum_alth.h Rawsock_lib/ipcsum_alth.c Rawsock_lib/minirighi_udp_checksum.h Rawsock_lib/minirighi_udp_checksum.c

Replacing "x86_64-openwrt-linux-musl-gcc" with the proper "gcc" binary.

**Headers to be included in your project**

You can include:
- rawsock.h, if you want to use the main Rawsock library module.
- rawsock_lamp.h, if you want to use the main Rawsock library module, with the additional _LaMP_ module.
- ipcsum_alth.h, only if you want to separately compute an IPv4 checksum in your application (normally, it is not needed)
- minirighi_udp_checksum.h, only if you want to separately compute a UDP checksum in your application (normally, it is not needed)
