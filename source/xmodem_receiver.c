#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "xmodem.h"
#include "xmodem_receiver.h"

// set timings:
static const uint32_t  READ_BLOCK_TIMEOUT      = 60000; // 60 seconds
static const uint32_t  C_TIMEOUT               = 3000; // 3 second
static const uint8_t   MAX_RETRIES             = 5; // max 5 retries for ACK

// callbacks
static xmodem_callback_read_data  callback_read_data  = NULL;
static xmodem_callback_write_data callback_write_data = NULL;
static xmodem_callback_set_buff   callback_set_buff   = NULL;

// global vars
static xmodem_receive_state_t receive_state    = XMODEM_RECEIVE_UNKNOWN;
static uint32_t        payload_buffer_position = 0;
static xmodem_packet_t current_packet          = {0};
static uint8_t         retries                 = 0;


xmodem_receive_state_t xmodem_receive_state()
{
   return receive_state;
}


bool xmodem_receive_init()
{
  
   bool result          = false; 
   receive_state        = XMODEM_RECEIVE_UNKNOWN;

   if (NULL != callback_read_data &&
       NULL != callback_write_data)
   {
      receive_state   = XMODEM_RECEIVE_INITIAL;
      // payload_buffer = (uint8_t *)malloc(XMODEM_PAYLOAD_SIZE + sizeof(xmodem_packet_t));
      result = true;
   }

   return result;
}

bool xmodem_receive_cleanup()
{
   //   callback_is_inbound_empty = 0;
   //   callback_is_outbound_full = 0;
   callback_read_data        = NULL;
   callback_write_data       = NULL;
   receive_state             = XMODEM_RECEIVE_UNKNOWN;
   payload_buffer_position   = 0;
//   if (payload_buffer)
//       free(payload_buffer);
//   payload_buffer            = 0;
//   inbound                   = 0;
//   returned_size             = 0;
//   control_character         = 0;
   retries                   = 0;
   
   return true;
}


xmodem_receive_state_t xmodem_receive_process(const uint32_t current_time)
{
   static uint32_t stopwatch = 0;   // here stored waiting start time to handle timeout

   switch(receive_state)
   {
      case XMODEM_RECEIVE_INITIAL:
      {
         receive_state = XMODEM_RECEIVE_SEND_C;
         retries = 0;

         break;
      }

      case XMODEM_RECEIVE_SEND_C:
      {
         if (callback_write_data(1, &(uint8_t)C))
         {
            receive_state = XMODEM_RECEIVE_WAIT_FOR_ACK;
            stopwatch = current_time; //
            retries = 0;
         }
         else  // something wrong with write_data callback - it should return true if send is okay.
         {
            if (retries < MAX_RETRIES)
            {
               retries++;
            }
            else
            {
               receive_state = XMODEM_RECEIVE_ABORT_TRANSFER;
            }
         }

         break;
      }

      case XMODEM_RECEIVE_WAIT_FOR_ACK:
      {
         
         if (current_time > (stopwatch + C_TIMEOUT)) //C_ACK timeout
         {
            receive_state = XMODEM_RECEIVE_TIMEOUT_ACK;
         }
         else
         {
            uint8_t input_byte;
            uint8_t returned_size = 0;

            if (callback_read_data(1, &input_byte, &returned_size))
            {
               if (returned_size > 0)
               {
                  if (ACK == input_byte)
                  {                   
                     receive_state = XMODEM_RECEIVE_ACK_SUCCESS;
                     break;
                  }
                  else if (EOT == input_byte)
                  {
                     receive_state = XMODEM_RECEIVE_TRANSFER_COMPLETE;
                     break;
                  }
               }
         }

         break;
      }

      case XMODEM_RECEIVE_TIMEOUT_ACK:
      {
         if (retries++ < MAX_RETRIES)
         {
            receive_state = XMODEM_RECEIVE_SEND_C;
         }
         else
         {
            receive_state = XMODEM_RECEIVE_ABORT_TRANSFER;
         }
         
         break;
      }

      case XMODEM_RECEIVE_ABORT_TRANSFER:
      {
         //TODO: implement final state
         break;
      }

      case XMODEM_RECEIVE_UNKNOWN:
      {
         receive_state = XMODEM_RECEIVE_ABORT_TRANSFER;
         break;
      }

      case XMODEM_RECEIVE_ACK_SUCCESS:
      {
         receive_state = XMODEM_RECEIVE_READ_BLOCK;
         stopwatch = current_time;
         retries = 0;

         break;
      }

      case XMODEM_RECEIVE_READ_:
      {

         break;
      }
      
      case XMODEM_RECEIVE_READ_BLOCK:
      {
#if ORIG
          if (current_time > (stopwatch + READ_BLOCK_TIMEOUT))
          {
             receive_state = XMODEM_RECEIVE_READ_BLOCK_TIMEOUT;
          }
          else
          {
             uint8_t   inbound      = 0;
             uint32_t  returned_size = 0;
 
             if (!callback_is_inbound_empty())
             {
                callback_read_data(1, &inbound, &returned_size);

                if (returned_size > 0)
                {
                   if (ACK == inbound)
                   {
                       receive_state = XMODEM_RECEIVE_ACK_SUCCESS;
                   }
                   else if (EOT == inbound)
                   {
                       receive_state = XMODEM_RECEIVE_TRANSFER_COMPLETE;
                   }
                } 
             } 

          }
#else
         if (current_time > (stopwatch + READ_BLOCK_TIMEOUT))
         {
           receive_state = XMODEM_RECEIVE_READ_BLOCK_TIMEOUT;
         }
         else
         {
            uint16_t max_bytes_to_read = XMODEM_PAYLOAD_SIZE - payload_buffer_position; // one byte (ACK) already on previous step received
            uint16_t readed_bytes_num = 0;

            if (callback_read_data(max_bytes_to_read, current_packet.payload, &payload_size))
            {

               receive_state = XMODEM_RECEIVE_READ_BLOCK_SUCCESS;
            }
         }
#endif

          break;
      }

      case XMODEM_RECEIVE_READ_BLOCK_TIMEOUT:
      {
          receive_state = XMODEM_RECEIVE_ABORT_TRANSFER;
          stopwatch = current_time;
          break;
      }

      case XMODEM_RECEIVE_READ_BLOCK_SUCCESS:
      {
          xmodem_packet_t *packet = (xmodem_packet_t *)payload_buffer;
                    
          uint16_t crc;
          if (xmodem_calculate_crc(packet->data, XMODEM_PAYLOAD_SIZE, &crc) && 
              (crc == packet->crc))
            receive_state = XMODEM_RECEIVE_BLOCK_VALID;
          else
            receive_state = XMODEM_RECEIVE_BLOCK_INVALID;

          break;
      }

      case XMODEM_RECEIVE_BLOCK_INVALID:
      {
         if (retries++ < MAX_RETRIES)
             receive_state = XMODEM_RECEIVE_READ_BLOCK;
         else
             receive_state = XMODEM_RECEIVE_ABORT_TRANSFER;
         break;
      }

      case XMODEM_RECEIVE_BLOCK_VALID:
      {
          receive_state = XMODEM_RECEIVE_BLOCK_ACK;
          break;
      }

      case XMODEM_RECEIVE_BLOCK_ACK:
      {
          //TODO: send ACK
          stopwatch = current_time; 
          receive_state = XMODEM_RECEIVE_WAIT_FOR_ACK;
          break;
      }

      default:
      {
          receive_state = XMODEM_RECEIVE_UNKNOWN; 
      }



   };

   return (receive_state);
    
}



void xmodem_receive_set_callback_write(xmodem_callback_write_data callback)
{
   callback_write_data = callback;
}

void xmodem_receive_set_callback_read(xmodem_callback_read_data callback)
{
   callback_read_data = callback;
}










