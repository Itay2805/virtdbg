#ifndef __VIRTDBG_SERIAL_H__
#define __VIRTDBG_SERIAL_H__

#include <stdbool.h>

/**
 * Init the serial driver
 */
void serial_init();

/**
 * Write a char to serial
 */
void serial_putc(char c);

/**
 * Get a char from serial
 */
char serial_getc();

/**
 * Called by trace for outputting characters
 */
void serial_output_cb(char c, void* ctx);

#endif
