#ifndef __VIRTDBG_TRACE_H__
#define __VIRTDBG_TRACE_H__

#include <sync/lock.h>
#include <stddef.h>
#include <stdarg.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Format utilities
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Callback to output a single character
 *
 * if the output is going to run out of space in 4 chars then this function should
 * return true, that will cause the printf to exit
 */
typedef void (*printf_callback_t)(char c, void* ctx);

/**
 * The most basic format function
 *
 * @param cb    [IN] Put char callback
 * @param ctx   [IN] The context of the callback
 * @param fmt   [IN] The format to print
 * @param ap    [IN] The va_list to read from
 */
size_t kvcprintf(printf_callback_t cb, void* ctx, const char* fmt, va_list ap);

/**
 * Format into a buffer
 *
 * @param buffer    [IN] The buffer to format into
 * @param size      [IN] The size of the buffer, including null terminator
 * @param fmt       [IN] The format string
 * @param ap        [IN] The va_list
 */
size_t kvsnprintf(char* buffer, size_t size, const char* fmt, va_list ap);

/**
 * Format into a buffer
 *
 * @param buffer    [IN] The buffer to format into
 * @param size      [IN] The size of the buffer, including null terminator
 * @param fmt       [IN] The format string
 */
size_t ksnprintf(char* buffer, size_t size, const char* fmt, ...);

/**
 * Printf function, outputs to the kernel debug output
 *
 * @param fmt   [IN] The format string
 */
size_t kprintf(const char* fmt, ...);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tracing utils
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------------------------------------
// These versions don't do any locks, and may cause output to be overlapped
//----------------------------------------------------------------------------------------------------------------------

#define UNLOCKED_DEBUG(fmt, ...)    kprintf("[?] " fmt "\n", ## __VA_ARGS__)
#define UNLOCKED_TRACE(fmt, ...)    kprintf("[*] " fmt "\n", ## __VA_ARGS__)
#define UNLOCKED_WARN(fmt, ...)     kprintf("[!] " fmt "\n", ## __VA_ARGS__)
#define UNLOCKED_ERROR(fmt, ...)    kprintf("[-] " fmt "\n", ## __VA_ARGS__)

//----------------------------------------------------------------------------------------------------------------------
// These versions use a lock, and make sure nothing will get overlapped
//----------------------------------------------------------------------------------------------------------------------

extern lock_t g_trace_lock;

#define DEBUG(...) \
    do { \
        lock(&g_trace_lock); \
        UNLOCKED_DEBUG(__VA_ARGS__); \
        unlock(&g_trace_lock); \
    } while(0)

#define TRACE(...) \
    do { \
        lock(&g_trace_lock); \
        UNLOCKED_TRACE(__VA_ARGS__); \
        unlock(&g_trace_lock); \
    } while(0)

#define WARN(...) \
    do { \
        lock(&g_trace_lock); \
        UNLOCKED_WARN(__VA_ARGS__); \
        unlock(&g_trace_lock); \
    } while(0)

#define ERROR(...) \
    do { \
        lock(&g_trace_lock); \
        UNLOCKED_ERROR(__VA_ARGS__); \
        unlock(&g_trace_lock); \
    } while(0)

#endif //__VIRTDBG_TRACE_H__
