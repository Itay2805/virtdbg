#include "gdb.h"

#include <arch/idt.h>
#include <drivers/serial.h>
#include <util/string.h>

/**
 * turn a number to a hex character
 */
static char m_hex_to_str[] = "0123456789ABCDEF";

static size_t str_to_hex(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('A' <= c && c <= 'F') {
        return c - 'A';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a';
    } else {
        WARN("Got invalid char when expecting hex (`%c`)", c);
        return -1;
    }
}

/**
 * Read hex from serial
 */
static size_t serial_read_hex(size_t count) {
    size_t num = 0;

    while (count--) {
        num <<= 4;
        num |= str_to_hex(serial_getc());
    }

    return num;
}

static size_t buf_read_hex(char* str) {
    size_t num = 0;

    while (str_to_hex(*str) != -1) {
        num <<= 4;
        num |= str_to_hex(*str);
        str++;
    }

    return num;
}

static void buf_write_hex(uint64_t num, char* str) {
    size_t off = 0;
    while (off < 64) {
        *str++ = m_hex_to_str[(num >> (off + 4)) & 0xF];
        *str++ = m_hex_to_str[(num >> off) & 0xF];
        off += 8;
    }
}


/**
 * Send a packet to the gdb client
 */
static void gdb_send_packet(char* packet_data) {
    // calculate the checksum
    uint8_t checksum = 0;
    for (char* ptr = packet_data; *ptr != '\0'; ptr++) {
        checksum += *ptr;
    }

    // try some times
    uint8_t retries = 10;
    do {
        if (retries-- == 0) {
            break;
        }

        // packet prefix
        serial_putc('$');

        // output the data
        for (char* ptr = packet_data; *ptr != '\0'; ptr++) {
            serial_putc(*ptr);
        }

        // output the checksum
        serial_putc('#');
        serial_putc(m_hex_to_str[checksum >> 4]);
        serial_putc(m_hex_to_str[checksum & 0xF]);
    } while(serial_getc() != '+');
}

/**
 * Receive a packet from the gdb client
 */
static err_t gdb_receive_packet(char* packet_data, size_t packet_data_size) {
    err_t err = NO_ERROR;
    uint8_t expected_checksum = 0;
    size_t off = 0;

    // check parameters
    CHECK(packet_data != NULL);

    // wait for the start of a packet
    while (serial_getc() != '$');

retry:
    // read the data itself
    expected_checksum = 0;
    off = 0;
    do {
        CHECK_ERROR(off < packet_data_size, ERROR_BUFFER_TOO_SMALL);

        // get the char until we get the checksum
        char c = serial_getc();
        if (c == '#') {
            break;
        }

        // calculate the checksum on the way while
        // filling the data
        expected_checksum += c;
        packet_data[off++] = c;
    } while(true);

    // put a zero terminator
    packet_data[off] = '\0';

    // check the checksum
    uint8_t checksum = serial_read_hex(2);
    if (checksum != expected_checksum) {
        WARN("gdb: Got invalid checksum, requesting packet again");

        // got invalid data for the checksum, tell the client
        // to resend it
        serial_putc('-');
        goto retry;
    } else {
        // ack that we got it
        serial_putc('+');
    }

cleanup:
    if (IS_ERROR(err)) {
        // send an error
        serial_putc('-');
    }
    return err;
}

static uint64_t* get_register_offset(exception_context_t* ctx, size_t reg) {
    switch (reg) {
        case 0: return &ctx->rax;
        case 1: return &ctx->rcx;
        case 2: return &ctx->rdx;
        case 3: return &ctx->rbx;
        case 4: return &ctx->rsp;
        case 5: return &ctx->rbp;
        case 6: return &ctx->rsi;
        case 7: return &ctx->rdi;
        case 8: return &ctx->rip;
        case 9: return (uint64_t*)&ctx->rflags;
        case 10: return &ctx->cs;
        case 11: return &ctx->ss;
        case 12: return &ctx->ds;
        case 13: return &ctx->ds; // es
        case 14: return &ctx->ds; // fs
        case 15: return &ctx->ds; // gs
        case 16: return &ctx->r8;
        case 17: return &ctx->r9;
        case 18: return &ctx->r10;
        case 19: return &ctx->r11;
        case 20: return &ctx->r12;
        case 21: return &ctx->r13;
        case 22: return &ctx->r14;
        case 23: return &ctx->r15;
        default: return NULL;
    }
}

#define SIGILL      4
#define SIGTRAP     5
#define SIGEMT      7
#define SIGFPE      8
#define SIGSEGV     11

static void send_signal(int sig) {
    char packet[4] = {3};
    char* ptr = packet;

    // set the trap
    *ptr++ = 'T';
    *ptr++ = m_hex_to_str[sig >> 4];
    *ptr++ = m_hex_to_str[sig & 0xF];

    // send it
    gdb_send_packet(packet);
}

static err_t gdb_exception_handler(exception_context_t* ctx, bool* handled) {
    err_t err = NO_ERROR;

    // remove single stepping
    ctx->rflags.TF = false;

    // send the exception code
    int sig = 0;
    switch (ctx->int_num) {
        case EXCEPT_DIVIDE_ERROR:   sig = SIGFPE; break;
        case EXCEPT_DEBUG:          sig = SIGTRAP; break;
        case EXCEPT_BREAKPOINT:     sig = SIGTRAP; break;
        case EXCEPT_INVALID_OPCODE: sig = SIGILL; break;
        case EXCEPT_DOUBLE_FAULT:   sig = SIGEMT; break;
        case EXCEPT_STACK_FAULT:    sig = SIGSEGV; break;
        case EXCEPT_GP_FAULT:       sig = SIGSEGV; break;
        case EXCEPT_PAGE_FAULT:     sig = SIGSEGV; break;
        case EXCEPT_FP_ERROR:       sig = SIGFPE; break;
        default: break;
    }

    // check if we handle this signal
    if (sig != 0) {
        *handled = true;
        goto cleanup;
    }

    // send that a signal happened
    send_signal(sig);

    // now handle any packet we get from gdb
    for (;;) {
        char data[256];
        CHECK_AND_RETHROW(gdb_receive_packet(data, sizeof(data)));

        // handle command
        switch (data[0]) {
            case '?': {
                //
            } break;

            case 'c': {
                // `c [addr]`
                // Continue at address, if no address is
                // provided just continue
                if (data[1] != '\0') {
                    ctx->rip = buf_read_hex(&data[1]);
                }
            } goto cleanup;

            case 'g': {
                // read general registers
                char buffer[24 * 16 + 1] = { 0 };
                for (int i = 0; i < 24; i++) {
                    buf_write_hex(*get_register_offset(ctx, i), &buffer[i * 8]);
                }
                gdb_send_packet(buffer);
            } break;

            case 'H': {
                // switch to another thread, we don't have any so
                // just return OK
                gdb_send_packet("OK");
            } break;

            case 's': {
                // `s [addr]`
                // Single step, if addr is specified resume at that address
                if (data[1] != '\0') {
                    ctx->rip = buf_read_hex(&data[1]);
                }
                ctx->rflags.TF = true;
            } break;

            default: {
                // send an empty packet to show this
                // is not supported
                gdb_send_packet("");
            } break;
        }
    }

cleanup:
    return err;
}

static exception_handler_t m_exception_handler = {
    .handle = gdb_exception_handler
};

void init_kernel_gdb() {
    hook_exception_handler(&m_exception_handler);
}
