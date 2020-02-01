/*
 * HttpSession.h
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */

#ifndef HTTPSESSION_H_
#define HTTPSESSION_H_

#include "Beast.h"
#include <boost/optional.hpp>

class HttpSession : public std::enable_shared_from_this<HttpSession>
{
public:
	HttpSession(tcp::socket&& socket);
    ~HttpSession()
    {
//    	std::cout << " ~HttpSession()" << std::endl;
    }

    void run();

private:
    void fail(beast::error_code ec, char const* what);
	template<class Body, class Allocator, class Send>
	void handleRequest(http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send);
    void doRead();
    void onRead(beast::error_code ec, std::size_t);
    void onWrite(beast::error_code ec, std::size_t, bool close);


    struct sendLambda;
    beast::tcp_stream _stream;
    beast::flat_buffer _buffer;

    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<http::request_parser<http::string_body>> _parser;
};


#endif /* HTTPSESSION_H_ */
