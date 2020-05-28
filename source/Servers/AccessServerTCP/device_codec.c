/*
 * device_codec.c
 *
 *  Created on: 23 Apr 2020
 *      Author: wenhe
 */

#include <arpa/inet.h>
#include "device_codec.h"
#include <string.h>

void rtio_message_init(rtio_message* message)
{
    memset((void*)(&message->header), 0, sizeof(rtio_message_header_t));
}

int rtio_message_header_decode(rtio_message* message)
{
    if(NULL == message)
    {
        return RTIO_CODE_MESSAGE_NULL;
    }
    message->header.version = (message->_buffer[0] >> 4) & 0x0F;
    message->header.type = message->_buffer[0] & 0x0F;
    message->header.id = (message->_buffer[1] << 8) + message->_buffer[2];
    message->header.body_length = (message->_buffer[3] << 8) + message->_buffer[4];
    return RTIO_CODE_SUCCESS;

}
int rtio_message_header_encode(rtio_message* message) // encode need after data assemble
{
    if(NULL == message)
    {
        return RTIO_CODE_MESSAGE_NULL;
    }
    message->_buffer[0] = ((message->header.version << 4) & 0xF0) + ((int) message->header.type & 0x0F);
    message->_buffer[1] = (message->header.id >> 8) & 0xFF; //msb
    message->_buffer[2] = message->header.id & 0xFF; //lsb
    if(message->header.body_length > RTIO_MESSAGE_BODY_LEN)
    {
        return RTIO_CODE_EXCEED_LENGTH;
    }
    message->_buffer[3] = (message->header.body_length >> 8) & 0xFF; //msb
    message->_buffer[4] = message->header.body_length & 0xFF; //lsb
    return RTIO_CODE_SUCCESS;
}
uint8_t* rtio_message_header(rtio_message* message)
{
    return message->_buffer;
}
uint16_t rtio_message_header_length(rtio_message* message)
{
    return RTIO_MESSAGE_HEADER_LEN;
}
uint8_t* rtio_message_body(rtio_message* message)
{
    return message->_buffer +  RTIO_MESSAGE_HEADER_LEN;
}
uint16_t rtio_message_body_length(rtio_message* message)
{
    return message->header.body_length;
}
uint16_t rtio_message_trans_length(rtio_message* message)
{
    return message->header.body_length + RTIO_MESSAGE_HEADER_LEN;
}


/* req field, req-data = body  */
uint8_t* rtio_message_get_from_body_req_data(rtio_message* message)
{
    return rtio_message_body(message);
}
uint16_t rtio_message_get_from_body_req_data_length(rtio_message* message)
{
    return message->header.body_length;
}
int rtio_message_assemble_body_req_data(rtio_message* message, uint8_t* data, uint16_t length)
{
    if(length > RTIO_MESSAGE_BODY_LEN)
    {
        return RTIO_CODE_EXCEED_LENGTH;
    }
    memcpy(rtio_message_body(message), data, length);
    message->header.body_length = length;
    return RTIO_CODE_SUCCESS;
}

/* resp field, byte0=resp-code, byte1|byte2=resp-data-length, byte3...=resp-data */
uint8_t rtio_message_get_from_body_resp_code(rtio_message* message)
{
    return *(message->_buffer +  RTIO_MESSAGE_HEADER_LEN);
}
uint8_t* rtio_message_get_from_body_resp_data(rtio_message* message)
{
    return rtio_message_body(message) + 3;
}
uint16_t rtio_message_get_from_body_resp_data_length(rtio_message* message)
{
    return (*(rtio_message_body(message) + 1) << 8) + *(rtio_message_body(message) + 2);
}
void rtio_message_assemble_body_resp_code(rtio_message* message, uint8_t code)
{
    *rtio_message_body(message) = code;
    if(message->header.body_length < 1) // if body_length bigger then resp.code length, do not set it
    {
        message->header.body_length = 1;
    }
}
int rtio_message_assemble_body_resp_data(rtio_message* message, uint8_t* data, uint16_t length)
{
    if(length > RTIO_MESSAGE_BODY_LEN - 3)
    {
        return RTIO_CODE_EXCEED_LENGTH;
    }

    *(rtio_message_body(message) + 1) = (length >> 8) & 0xFF;
    *(rtio_message_body(message) + 2) = length & 0xFF;

    memcpy(rtio_message_get_from_body_resp_data(message), data, length);
    message->header.body_length += 3 + length;
    return RTIO_CODE_SUCCESS;
}


//uint8_t* rtio_message_buffer(rtio_message* message)
//{
//    return message->_buffer;
//}
//uint16_t rtio_message_buffer_length(rtio_message* message)
//{
//    return RTIO_MESSAGE_BUFFER_LEN;
//}

