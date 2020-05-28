#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "CodecAdapter.hpp"

#include "Common.h"
#include "string.h"
#include <iostream>
using boost::asio::ip::tcp;
//void testDproto()
//{
//    DProto::Message m1;
//    m1.setId((uint16_t)0x8899);
//    m1.setVersion(0x01);
//    m1.setType(DProto::Type::REPORT_REQ);
//
//    std::string body = "good good bangbangbang";
//    m1.assembleBody_RespData((uint8_t*)body.c_str(), (uint16_t)body.length());
//
//    m1.encodeHeader();
//
//    std::cout << Util::binToHex(std::string_view((char*)m1.header(), m1.headerLength()+m1.bodyLength())) << std::endl;
//
//
//
//    DProto::Message m2;
//    memcpy(m2.header(), m1.header(),  m1.bodyLength() + m1.headerLength());
//    m2.decodeHeader();
//    std::cout << std::hex << m2.id() << std::endl;
//    std::cout << m2.bodyLength() << std::endl;
//    std::cout << m2.version() << std::endl;
//    std::cout << (int)m2.type() << std::endl;
//
//
//}

class chat_client
{
public:
    chat_client(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints) :
            io_context_(io_context), socket_(io_context)
    {
        do_connect(endpoints);
    }

    void write(const DProto::Message& msg)
    {
        boost::asio::post(io_context_, [this, msg]()
        {
            bool write_in_progress = !write_msgs_.empty();
            write_msgs_.push_back(msg);
            if (!write_in_progress)
            {
                do_write();
            }
        });
    }

    void close()
    {
        boost::asio::post(io_context_, [this]()
        {   socket_.close();});
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints)
    {
        boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, tcp::endpoint)
        {
            if (!ec)
            {
                do_read_header();
            }
        });
    }

    void do_read_header()
    {
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.header(), read_msg_.headerLength()),
                [this](boost::system::error_code ec, std::size_t /*length*/)
                {
                    std::cout << "###3 do_read_header ec=" << ec << std::endl;
                    if (!ec)
                    {
                        if(read_msg_.decodeHeader() != DProto::RetCode::SUCCESS)
                        {
                            socket_.close();
                        }
                        else
                        {
                            do_read_body();
                        }

                    }
                    else
                    {
                        socket_.close();
                    }
                });
    }

    void do_read_body()
    {
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.body(), read_msg_.bodyLength()),
                [this](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (!ec)
                    {
                        std::cout << "RESP=" << Util::binToHex(std::string_view((char*)read_msg_.body(), read_msg_.bodyLength()));
                        std::cout << "\n";

                        switch(read_msg_.type())
                        {
                            case DProto::Type::REPORT_RESP:
                            {
                                std::cout << "REPORT_RESP=" << std::hex << (uint32_t) *read_msg_.body() <<  std::endl;
                                break;
                            }
                            case DProto::Type::DISPATCH_REQ:
                            {

                                std::cout << "DISPATCH_REQ=" << std::string_view((char*)read_msg_.body(), read_msg_.bodyLength()) <<  std::endl;

                                // response resp
                                DProto::Message m1;
                                m1.setId((uint16_t) read_msg_.id());
                                m1.setVersion(0x01);
                                m1.setType(DProto::Type::DISPATCH_RESP);

                                std::string data = "this a dada";
                                m1.assembleBody_RespCode(DProto::RemoteCode::SUCCESS);
                                m1.assembleBody_RespData((uint8_t*)data.c_str(),(uint16_t)data.length());

                                m1.encodeHeader();
                                write(m1);
                                break;// todo need a logic
                            }
                            case DProto::Type::AUTH_RESP:
                                std::cerr << "AUTH_RESP" << std::endl;
                                break;
                            default:
                            std::cerr << "message type error" << std::endl;
                            break;

                        }


                        do_read_header();
                    }
                    else
                    {
                        socket_.close();
                    }
                });
    }

    void do_write()
    {
        auto m = write_msgs_.front();
        boost::asio::async_write(socket_, boost::asio::buffer(m.header(), m.headerLength() + m.bodyLength()),
                [this](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (!ec)
                    {
                        write_msgs_.pop_front();
                        if (!write_msgs_.empty())
                        {
                            do_write();
                        }
                    }
                    else
                    {
                        socket_.close();
                    }
                });
    }

private:
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    DProto::Message read_msg_;
    std::deque<DProto::Message> write_msgs_;
};




int tcpClient()
{
    try
    {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");
        chat_client c(io_context, endpoints);

        std::thread t([&io_context](){   io_context.run();});
        // auth
        {
            DProto::Message m1;
            m1.setId((uint16_t) 0x8899);
            m1.setVersion(0x01);
            m1.setType(DProto::Type::AUTH_REQ);

//            std::string authData = "63eb0819f6654063bd3be8cc03f415bf:yePKI+jFTjEuy3XtU9RQgpt7xDI";
//            std::string authData = "b1a54328c072456bbd441abe996a8e3a:zJu-FdDvunWxEkmTY5Y6e9FD8jY";
            std::string authData = "4ffffb282b774b1fa1942b76445131d7:vzidbk0RklWOZZlmx6geBwhA2P0";
            m1.assembleBody_ReqData((uint8_t*)authData.c_str(), authData.length());
            m1.encodeHeader();
            c.write(m1);
        }

        char line[1025];
        while(std::cin.getline(line, 1025))
        {
            DProto::Message m1;
            m1.setId((uint16_t) 0x889a);
            m1.setVersion(0x01);
            m1.setType(DProto::Type::REPORT_REQ);
            m1.assembleBody_ReqData((uint8_t*)line, std::strlen(line));
            m1.encodeHeader();
            c.write(m1);
        }

        c.close();
        t.join();
    }
    catch(std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}


int main()
{
    std::cout << "###2" << std::endl;
    tcpClient();
    return 0;
}
