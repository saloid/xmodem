#ifndef XMODEM_H_
#define XMODEM_H_

#include <stdint.h>

enum XMODEM_CONTROL_CHARACTERS {SOH = 0x01, EOT = 0x04, ACK = 0x06, NACK = 0x15, ETB = 0x17, CAN = 0x18, C = 0x43}; 

#define XMODEM_PAYLOAD_SIZE 128
#define XMODEM_FULL_PACKET_SIZE (XMODEM_PAYLOAD_SIZE + 5)

//static const uint8_t  XMODEM_PAYLOAD_SIZE  = 128;   // fixed block size 

#ifdef __GNUC__
typedef struct
{
  uint8_t  preamble;
  uint8_t  id;
  uint8_t  id_complement;
  uint8_t  data[XMODEM_PAYLOAD_SIZE];
  uint16_t crc;
} xmodem_packet_t __attribute__((__packed__));
#else
#pragma pack(push,1)
typedef struct
{
  uint8_t  preamble;
  uint8_t  id;
  uint8_t  id_complement;
  uint8_t  data[XMODEM_PAYLOAD_SIZE];
  uint16_t crc;
} xmodem_packet_t __attribute__((__packed__));
#pragma pack(pop)
#endif

// Read input data in this callback. Returned value is true if readed at least 1 byte readed
typedef bool (*xmodem_callback_read_data)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size);

// Send data in this callback. Returned value and write_success are true if write succefull
typedef bool (*xmodem_callback_write_data)(const uint32_t requested_size, uint8_t *buffer, bool *write_success);

bool xmodem_verify_packet(const xmodem_packet_t packet, uint8_t expected_packet_id);
bool xmodem_calculate_crc(const uint8_t *data, const uint32_t size, uint16_t *result);

#endif // XMODEM_H_
