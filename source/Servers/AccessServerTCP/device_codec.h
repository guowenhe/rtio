/*
 * device_codec.h
 *
 *  Created on: 23 Apr 2020
 *      Author: wenhe
 */

#ifndef PROTO_CODEC_DEVICE_DEVICE_CODEC_H_
#define PROTO_CODEC_DEVICE_DEVICE_CODEC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define RTIO_DEVICE_CODEC_VERSION 1

#define RTIO_MESSAGE_HEADER_LEN     5
#define RTIO_MESSAGE_BODY_LEN       1024
#define RTIO_MESSAGE_BUFFER_LEN     (RTIO_MESSAGE_HEADER_LEN + RTIO_MESSAGE_BODY_LEN)

#define RTIO_MESSAGE_TYPE_UNSET             0
#define RTIO_MESSAGE_TYPE_REPORT_REQ        1
#define RTIO_MESSAGE_TYPE_REPORT_RESP       2
#define RTIO_MESSAGE_TYPE_DISPATCH_REQ      3
#define RTIO_MESSAGE_TYPE_DISPATCH_RESP     4
#define RTIO_MESSAGE_TYPE_AUTH_REQ          5
#define RTIO_MESSAGE_TYPE_AUTH_RESP         6

#define RTIO_CODE_SUCCESS                   0
#define RTIO_CODE_FAIL                      -1
#define RTIO_CODE_EXCEED_LENGTH             -2
#define RTIO_CODE_MESSAGE_NULL              -3

#define RTIO_REMOTE_CODE_SUCCESS            0x10
#define RTIO_REMOTE_CODE_AUTH_PASS          0x11
#define RTIO_REMOTE_CODE_AUTH_FAIL          0x12
#define RTIO_REMOTE_CODE_AUTH_DATA_INVALID  0x13
#define RTIO_REMOTE_CODE_TYPE_ERROR         0x14
#define RTIO_REMOTE_CODE_SERVER_ERROR       0xFF


typedef struct rtio_message_header_t
{
    uint8_t version;
    uint8_t type;
    uint16_t id;
    uint16_t body_length;
} rtio_message_header_t;

typedef struct rtio_message
{
    rtio_message_header_t header;
    uint8_t _buffer[RTIO_MESSAGE_BUFFER_LEN];

} rtio_message;

void rtio_message_init(rtio_message* message);
int rtio_message_header_decode(rtio_message* message);
int rtio_message_header_encode(rtio_message* message);  // encode need after data assemble
uint8_t* rtio_message_header(rtio_message* message);
uint16_t rtio_message_header_length(rtio_message* message);
uint8_t* rtio_message_body(rtio_message* message);
uint16_t rtio_message_body_length(rtio_message* message);
uint16_t rtio_message_trans_length(rtio_message* message);

/* req field, req-data = body  */
int rtio_message_assemble_body_req_data(rtio_message* message, uint8_t* data, uint16_t length);
uint8_t* rtio_message_get_from_body_req_data(rtio_message* message);
uint16_t rtio_message_get_from_body_req_data_length(rtio_message* message);
/* resp field, byte0=resp-code, byte1|byte2=resp-data-length, byte3...=resp-data */
uint8_t rtio_message_get_from_body_resp_code(rtio_message* message);
uint8_t* rtio_message_get_from_body_resp_data(rtio_message* message);
uint16_t rtio_message_get_from_body_resp_data_length(rtio_message* message);
void rtio_message_assemble_body_resp_code(rtio_message* message, uint8_t code);
int rtio_message_assemble_body_resp_data(rtio_message* message, uint8_t* data, uint16_t length);

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* PROTO_CODEC_DEVICE_DEVICE_CODEC_H_ */
