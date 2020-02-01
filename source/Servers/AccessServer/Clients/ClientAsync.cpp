//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket client, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/signal_set.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include "MCodec.hpp"

namespace beast = boost::beast;
// from <boost/beast.hpp>
namespace http = beast::http;
// from <boost/beast/http.hpp>
namespace websocket = beast::websocket;
// from <boost/beast/websocket.hpp>
namespace net = boost::asio;
// from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;
// from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session: public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;

public:
    // Resolver and socket require an io_context
    explicit session(net::io_context& ioc) :
            resolver_(net::make_strand(ioc)), ws_(net::make_strand(ioc))
    {
    }
    ~session()
    {
    }

    // Start the asynchronous operation
    void run(char const* host, char const* port, char const* text)
    {
        // Save these for later
        host_ = host;
        text_ = text;

        // Look up the domain name
        resolver_.async_resolve(host, port, beast::bind_front_handler(&session::on_resolve, shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        // Set the timeout for the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(results,
                beast::bind_front_handler(&session::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
    {
        if(ec)
            return fail(ec, "connect");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();

        // Set suggested timeout settings for the websocket
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator([](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-async");
        }));

        // Perform the websocket handshake
        ws_.async_handshake(host_, "/auth?deviceid=" + text_,
                beast::bind_front_handler(&session::on_handshake, shared_from_this()));
    }

    void on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        // Send the message

        MCodec::Req req;
        req.type = MCodec::Type::reportReq;
        req.id = 10;
        req.content = "this message from device";
        std::string package;
        MCodec::serial(req, package);

        ws_.binary(true);
        ws_.async_write(net::buffer(package), beast::bind_front_handler(&session::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Read a message into our buffer
        ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        std::string binary(beast::buffers_to_string(buffer_.data()));

        switch(MCodec::headType(binary))
        {
            case MCodec::Type::reportResp:
            {
                MCodec::Resp resp;
                MCodec::parse(binary, resp);
                std::cout << MCodec::debugShowResp(resp, "resp")<<std::endl;
                break;
            }
            case MCodec::Type::dispatchReq:
            {
                MCodec::Req req;
                MCodec::parse(binary, req);
                std::cout << MCodec::debugShowReq(req, "req")<<std::endl;


                // response resp
                MCodec::Resp dispatchResp;
                dispatchResp.type = MCodec::Type::dispatchResp;
                dispatchResp.id = req.id;
                dispatchResp.code = 0;
                std::string respBinary;
                MCodec::serial(dispatchResp, respBinary);
                ws_.binary(true);
                ws_.async_write(net::buffer(respBinary), beast::bind_front_handler(&session::on_write, shared_from_this()));
                buffer_.consume(buffer_.size());
                return; // todo need a logic
            }
            default:
                std::cerr << "message type error" << std::endl;
                break;

        }



//        std::cout << "read:" << (long) this << ":" << beast::make_printable(buffer_.data()) << std::endl;

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Read another message
        ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read, shared_from_this()));

    }

    void close()
    {
        // Post our work to the strand, this ensures
        // that the members of `this` will not be
        // accessed concurrently.

        net::post(ws_.get_executor(), beast::bind_front_handler(&session::do_close, shared_from_this()));
        std::cout << "##close:" << std::endl;

    }
    void do_close()
    {
        // Close the WebSocket connection
        ws_.async_close(websocket::close_code::normal,
                beast::bind_front_handler(&session::on_close, shared_from_this()));
        std::cout << "##do_close:" << std::endl;
    }

    void on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << "on_close:" << beast::make_printable(buffer_.data()) << std::endl;
    }
};

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Check command line arguments.
    if(argc != 4)
    {
        std::cerr << "Usage: websocket-client-async <host> <port> <text>\n" << "Example:\n"
                << "    websocket-client-async echo.websocket.org 80 \"Hello, world!\"\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const text = argv[3];

    // The io_context is required for all I/O
    net::io_context ioc;

    // Launch the asynchronous operation
    auto sesson1 = std::make_shared<session>(ioc);
    std::cout << "s1:" << (long) sesson1.get() << std::endl;
    sesson1->run(host, port, text);
//    auto sesson2 = std::make_shared<session>(ioc);
//    std::cout << "s1:" << (long) sesson1.get() << std::endl;
//    sesson2->run(host, port, "this session2");

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc, sesson1](boost::system::error_code const&, int)
    {
        // Stop the io_context. This will cause run()
        // to return immediately, eventually destroying the
        // io_context and any remaining handlers in it.
//            ioc.stop();

            sesson1->close();
        });
//    net::signal_set signals(ioc, SIGINT, SIGTERM);
//    signals.async_wait(
//        [&ioc, sesson1, sesson2](boost::system::error_code const&, int)
//        {
//            // Stop the io_context. This will cause run()
//            // to return immediately, eventually destroying the
//            // io_context and any remaining handlers in it.
////            ioc.stop();
//
//    	    sesson1->close();
//    	    sesson2->close();
//        });

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}
