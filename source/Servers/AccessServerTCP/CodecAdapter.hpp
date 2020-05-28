/*
 * CodecAdapter.hpp
 *
 *  Created on: 25 Apr 2020
 *      Author: wenhe
 */

#ifndef CODECADAPTER_HPP_
#define CODECADAPTER_HPP_

#include "device_codec.h"

namespace DProto // device protocal
{

enum class Type
{
    UNSET           = 0,
    REPORT_REQ      = 1,
    REPORT_RESP     = 2,
    DISPATCH_REQ    = 3,
    DISPATCH_RESP   = 4,
    AUTH_REQ        = 5,
    AUTH_RESP       = 6,
};

enum class RemoteCode: uint8_t
{
    SUCCESS             = RTIO_REMOTE_CODE_SUCCESS,
    AUTH_PASS           = RTIO_REMOTE_CODE_AUTH_PASS,
    AUTH_FAIL           = RTIO_REMOTE_CODE_AUTH_FAIL,
    AUTH_DATA_INVALID   = RTIO_REMOTE_CODE_AUTH_DATA_INVALID,
    TYPE_ERROR          = RTIO_REMOTE_CODE_TYPE_ERROR,
    SERVER_FAIL         = RTIO_REMOTE_CODE_SERVER_ERROR,
};

enum class RetCode
{
    SUCCESS             = RTIO_CODE_SUCCESS,
    FAIL                = RTIO_CODE_FAIL,
    EXCEED_LENGTH       = RTIO_CODE_EXCEED_LENGTH,
    MESSAGE_NULL        = RTIO_CODE_MESSAGE_NULL,
};



class Message
{
public:
    Message()
    {
        rtio_message_init(&_message);
    }

    uint8_t currentVersion()
    {
        return RTIO_DEVICE_CODEC_VERSION;
    }

    RetCode decodeHeader()
    {
        return static_cast<RetCode>(rtio_message_header_decode(&_message));
    }
    RetCode encodeHeader()  //encode need after data assemble
    {
        return static_cast<RetCode>(rtio_message_header_encode(&_message));
    }

    int version()
    {
        return _message.header.version;
    }
    int id()
    {
        return _message.header.id;
    }
    Type type()
    {
        return static_cast<Type>(_message.header.type);
    }
    void setVersion(uint8_t version)
    {
        _message.header.version = version;
    }
    void setId(uint16_t id)
    {
        _message.header.id = id;
    }
    void setType(Type type)
    {
        _message.header.type = static_cast<int>(type);
    }

    uint8_t* header()
    {
        return rtio_message_header(&_message);
    }
    int headerLength()
    {
        return rtio_message_header_length(&_message);
    }
    uint8_t* body()
    {
        return rtio_message_body(&_message);
    }
    int bodyLength()
    {
        return _message.header.body_length;
    }
    int transLength()
    {
        return rtio_message_trans_length(&_message);
    }

    // req functions
    uint8_t* getFromBody_ReqData()
    {
        return rtio_message_get_from_body_req_data(&_message);
    }
    uint16_t getFromBody_ReqDataLength()
    {
        return rtio_message_get_from_body_resp_data_length(&_message);
    }
    RetCode assembleBody_ReqData(uint8_t* data, uint16_t length)
    {
        return static_cast<RetCode>(rtio_message_assemble_body_req_data(&_message, data, length));
    }
    // resp functions
    RemoteCode getFromBody_RespCode()
    {
        return static_cast<RemoteCode>(rtio_message_get_from_body_resp_code(&_message));
    }
    uint8_t* getFromBody_RespData()
    {
        return rtio_message_get_from_body_resp_data(&_message);
    }
    uint16_t getFromBody_RespDataLength()
    {
        return rtio_message_get_from_body_resp_data_length(&_message);
    }
    void assembleBody_RespCode(RemoteCode code)
    {
        rtio_message_assemble_body_resp_code(&_message, static_cast<uint8_t>(code));
    }
    void assembleBody_RespData(uint8_t* data, uint16_t length)
    {
        rtio_message_assemble_body_resp_data(&_message, data, length);
    }

private:
    rtio_message _message;
};




} //Codec

#endif /* CODECADAPTER_HPP_ */
