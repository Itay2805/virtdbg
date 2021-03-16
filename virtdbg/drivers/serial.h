#ifndef __VIRTDBG_SERIAL_H__
#define __VIRTDBG_SERIAL_H__

void serial_init();
void serial_output_cb(char c, void* ctx);

#endif
