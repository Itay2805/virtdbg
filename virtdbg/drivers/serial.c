#include <drivers/serial.h>
#include <arch/io.h>
#include <stdint.h>

#define SERIAL_BASE 0x3F8
#define LSR         (SERIAL_BASE + 0x05)
#define TXRDY       0x20

void serial_init() {
    io_write_8(SERIAL_BASE + 3, 3); // Configure Line Control
    io_write_8(SERIAL_BASE + 1, 0); // Disable IRQs

    uint16_t divisor = 3;
    io_write_8(SERIAL_BASE + 3, io_read_8(SERIAL_BASE + 3) | (1 << 7)); // Set DLAB
    io_write_8(SERIAL_BASE, (divisor >> 8) & 0xFF);
    io_write_8(SERIAL_BASE, divisor & 0xFF);
    io_write_8(SERIAL_BASE + 3, io_read_8(SERIAL_BASE + 3) & ~(1 << 7)); // Clear DLAB
}

void serial_output_cb(char c, void* ctx) {
    uint8_t data;
    do {
        data = io_read_8(LSR);
    } while(!(data & TXRDY));
    io_write_8(SERIAL_BASE, c);
}
