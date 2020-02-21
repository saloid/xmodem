#ifndef XMODEM_RECEIVER_H_
#define XMODEM_RECEIVER_H_

#include "xmodem.h"

enum XMODEM_RECEIVE_STATES {XMODEM_RECEIVE_INITIAL,
                            XMODEM_RECEIVE_SEND_C,                    XMODEM_RECEIVE_WAIT_FOR_ACK,
                            XMODEM_RECEIVE_TIMEOUT_ACK,               XMODEM_RECEIVE_READ_BLOCK_TIMEOUT,
                            XMODEM_RECEIVE_ABORT_TRANSFER,            XMODEM_RECEIVE_READ_BLOCK,
                            XMODEM_RECEIVE_ACK_SUCCESS,               XMODEM_RECEIVE_TRANSFER_COMPLETE,
                            XMODEM_RECEIVE_READ_BLOCK_SUCCESS,        XMODEM_RECEIVE_BLOCK_INVALID,
                            XMODEM_RECEIVE_BLOCK_ACK,                 XMODEM_RECEIVE_BLOCK_VALID,
                            XMODEM_RECEIVE_UNKNOWN } typedef xmodem_receive_state_t;

// Set payload buffer. This callback triggered in next cases: at first time, after buffer full. Should fit single payload packet size (XMODEM_PAYLOAD_SIZE)
typedef bool (*xmodem_callback_set_buff)(const uint32_t requested_size, uint8_t *buffer, uint32_t *provided_buffer_size);


xmodem_receive_state_t xmodem_receive_state();

bool xmodem_receive_init();
xmodem_receive_state_t xmodem_receive_process(const uint32_t current_time);
bool xmodem_receive_cleanup();

void xmodem_receive_set_callback_write(xmodem_callback_write_data callback);
void xmodem_receive_set_callback_read(xmodem_callback_read_data callback);

#endif // XMODEM_RECEIVER_H_
