/*
 * MCodec.hpp
 *
 *  Created on: Dec 25, 2019
 *      Author: wenhe
 */

#ifndef MCODEC_HPP_
#define MCODEC_HPP_

#include <sstream>
#include <string>
#include <iomanip>

namespace MCodec //message codec
{

enum class Type
{
    unSet = 0,
    reportReq = 1,
    reportResp = 2,
    dispatchReq = 3,
    dispatchResp = 4,
};
std::ostream& operator<<(std::ostream& out, Type type)
{
    switch(type)
    {
    case Type::reportReq:
        out << "reportReq";
        break;
    case Type::reportResp:
        out << "reportResp";
        break;
    case Type::dispatchReq:
        out << "dispatchReq";
        break;
    case Type::dispatchResp:
        out << "dispatchResp";
        break;
    default:
        out << "no-desc(" << static_cast<int>(type) << ")";
    }
    return out;
}

struct Req
{
    uint8_t version = 1;
    Type type = Type::unSet;
    uint16_t id = 0;
    std::string content;
};
struct Resp
{
    uint8_t version = 1;
    Type type = Type::unSet;
    uint16_t id = 0;
    uint8_t code = -1;
};

constexpr uint16_t u16Cast(char c)
{
    return (uint16_t)(c) & 0x00FF;
}


void serial(const Req& req, std::string& binary)
{
    std::string header;
    header.resize(5);
    header[0] = ((req.version << 4) & 0xF0) + ((int)req.type & 0x0F);
    header[1] = (req.id >> 8) & 0xFF; //msb
    header[2] = req.id & 0xFF; //lsb
    header[3] = (req.content.size() >> 8) & 0xFF; //msb
    header[4] = req.content.size() & 0xFF; //lsb
    binary.insert(0, header, 0, 5);
    binary.insert(5, req.content, 0, req.content.length());
}
int parse(const std::string& binary, Req& req)
{
    if(binary.length() < 5)
    {
        return -1;
    }
    req.version = (u16Cast(binary[0]) >> 4) & 0x0F;
    req.type = (Type)(u16Cast(binary[0]) & 0x0F);
    req.id = (u16Cast(binary[1]) << 8) + u16Cast(binary[2]);
    req.content.insert(0, binary, 5, binary.length() - 5);
    return 0;
}

void serial(const Resp& resp, std::string& binary)
{
    std::string header;
    header.resize(4);
    header[0] = ((resp.version << 4) & 0xF0) + ((int)resp.type & 0x0F);
    header[1] = (resp.id >> 8) & 0xFF; //msb
    header[2] = resp.id & 0xFF; //lsb
    header[3] = resp.code;
    binary.insert(0, header, 0, 4);
}
int parse(const std::string& binary, Resp& resp)
{
    if(binary.length() < 4)
    {
        return -1;
    }
    resp.version = (u16Cast(binary[0]) >> 4) & 0x0F;
    resp.type = (Type)(u16Cast(binary[0]) & 0x0F);
    resp.id = (u16Cast(binary[1]) << 8) + u16Cast(binary[2]);
    resp.code = u16Cast(binary[3]);
    return 0;
}
Type headType(const std::string& binary)
{
    if(binary.length() < 1)
    {
        return Type::unSet;
    }
    return (Type)(binary[0] & 0x0F);
}
int headVersion(const std::string& binary)
{
    if(binary.length() < 1)
    {
        return -1;
    }
    return (binary[0] >> 4) & 0x0F;
}

std::string debugShowHex(const std::string& binary, const std::string name)
{
    std::stringstream ss;
    ss << name << " hex:" << std::endl;
    for(int i = 0; i < binary.length(); ++i)
    {
        ss <<  std::hex << std::setw(2) << std::setfill('0') << ((uint32_t)binary[i] & 0xFF) << " ";
        if((i + 1) % 16 == 0)
        {
            ss << std::endl;
        }
        else if((i + 1) % 8 == 0)
        {
            ss << " ";
        }
    }
    return ss.str();
}
std::string debugShowReq(const Req& req, const std::string name)
{
    std::stringstream ss;
    ss << name << std::endl;
    ss << "{" << std::endl;
    ss << "    version: " << (uint32_t)req.version << std::endl;
    ss << "    type: " << req.type << std::endl;
    ss << "    id: " << (uint32_t)req.id << std::endl;
    ss << "    content: " << req.content << std::endl;
    ss << "}" << std::endl;
    return ss.str();
}
std::string debugShowResp(const Resp& resp,  const std::string name)
{
    std::stringstream ss;
    ss << name << std::endl;
    ss << "{" << std::endl;
    ss << "    version: " << (uint32_t)resp.version << std::endl;
    ss << "    type: " << resp.type << std::endl;
    ss << "    id: " << (uint32_t)resp.id << std::endl;
    ss << "    code: " << (uint32_t)resp.code << std::endl;
    ss << "}" << std::endl;
    return ss.str();
}


}




#endif /* MCODEC_HPP_ */
