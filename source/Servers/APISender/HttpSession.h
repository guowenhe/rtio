/*
 * HttpSession.h
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */

#ifndef HTTPSESSION_H_
#define HTTPSESSION_H_

#include <boost/optional.hpp>
#include "Beast.h"
#include "IceClient.h"

struct HttpRespInfo
{
    unsigned version;
    bool keepAlive;
    http::status status;
    std::string body;
    GlobalResources::SendPara para;
};

const std::string uriSend("/call/send");

class HttpSession : public std::enable_shared_from_this<HttpSession>
{
public:
	HttpSession(tcp::socket&& socket);
	~HttpSession();

    void run();

private:
    void fail(beast::error_code ec, char const* what);
	template<class Body, class Allocator>
	void handleRequest(http::request<Body, http::basic_fields<Allocator>>&& req);

	void responseEx(std::shared_ptr<HttpRespInfo> info, std::shared_ptr<GlobalResources::SendReturn> ret);
	void response(std::shared_ptr<HttpRespInfo> info, std::shared_ptr<GlobalResources::SendReturn> ret  = nullptr);//, beast::error_code ec, std::size_t);

    void doRead();
    void onRead(beast::error_code ec, std::size_t);
    void onWrite(bool close, beast::error_code ec, std::size_t);//beast::error_code ec, std::size_t, bool close);


//    struct sendLambda;

    beast::tcp_stream _stream;
    beast::flat_buffer _buffer;

    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<http::request_parser<http::string_body>> _parser;
    const std::string _serverField = "rtio/0.1";
};


#endif /* HTTPSESSION_H_ */
