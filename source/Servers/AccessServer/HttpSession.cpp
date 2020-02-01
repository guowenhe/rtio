/*
 * HttpSession.cpp
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */
#include "VerijsLog.h"
#include "IceClient.h"
#include "HttpSession.h"
#include "WebsocketSession.h"

#define HTTP_KEY_INFO "deviceid="

struct HttpSession::sendLambda
{
	HttpSession &_self;

	explicit sendLambda(HttpSession &self) :
			_self(self)
	{
	}

	template<bool isRequest, class Body, class Fields>
	void operator()(http::message<isRequest, Body, Fields> &&msg) const
	{
		// The lifetime of the message has to extend
		// for the duration of the async operation so
		// we use a shared_ptr to manage it.
		auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

		// Write the response
		auto self = _self.shared_from_this();
		http::async_write(_self._stream, *sp, [self, sp](beast::error_code ec, std::size_t bytes)
		{
			self->onWrite(ec, bytes, sp->need_eof());
		});
	}
};

template<class Body, class Allocator, class Send>
void HttpSession::handleRequest(http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send)
{
	// Returns a bad request response
	auto const badRequest = [&req](beast::string_view why)
	{
		http::response<http::string_body> res
		{
			http::status::bad_request,
			req.version()
		};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = std::string(why);
		res.prepare_payload();
		return res;
	};

	// Returns a not found response
	auto const notFound = [&req](beast::string_view target)
	{
		http::response<http::string_body> res
		{
			http::status::not_found,
			req.version()
		};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "The resource '" + std::string(target) + "' was not found.";
		res.prepare_payload();
		return res;
	};

	// Returns a server error response
	auto const serverError = [&req](beast::string_view what)
	{
		http::response < http::string_body > res
		{
			http::status::internal_server_error,
			req.version()
		};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "An error occurred: '" + std::string(what) + "'";
		res.prepare_payload();
		return res;
	};

	// Make sure we can handle the method
	if(req.method() != http::verb::get)
	{
		 send(badRequest("Unknown HTTP-method"));
		 return;
	}

	// todo check auth data valid?
	// valid data, call auth and set auth-cb
	// todo using constexpr
	// /auth?deviceid=xxxx&devicekey=xxxx
	std::cout << "req.target()=" << req.target() << std::endl;
    if(req.target().size() < 6 ||
            req.target()[0] != '/'||
            req.target()[1] != 'a'||
            req.target()[2] != 'u'||
            req.target()[3] != 't'||
            req.target()[4] != 'h'||
            req.target()[5] != '?')
    {
        send(badRequest("Illegal request-target"));
        return;
    }

    auto posId = req.target().find(HTTP_KEY_INFO);
    if(posId == beast::string_view::npos)
    {
        send(badRequest("Illegal request-target"));
        return;
    }
    auto posVal = posId + sizeof(HTTP_KEY_INFO) - 1;
    const std::string deviceId = static_cast<std::string>(req.target().substr(posVal, req.target().size() - posVal));

    std::cout << "deviceId=" << deviceId << std::endl;

	//  todo mv to auth-cb
	// Request path must be absolute and not contain "..".
	if(websocket::is_upgrade(req))
	{
//		if (req.base().target().find(HTTP_KEY_INFO) == beast::string_view::npos) // as auth failed
//		{
//			badRequest("Auth failed");
//			return;
//		}
		std::make_shared<WebsocketSession>(_stream.release_socket(), deviceId)->run(req);
	}

}

void HttpSession::fail(beast::error_code ec, char const *what)
{
	// Don't report on canceled operations
	if(ec == asio::error::operation_aborted)
		return;

	auto *iceClient = GlobalResources::IceClient::getInstance();
	log2E(iceClient->getCommunicator(), what << ": " << ec.message());
}

HttpSession::HttpSession(tcp::socket &&socket) :
		_stream(std::move(socket))
{
}

void HttpSession::run()
{
	doRead();
}

void HttpSession::doRead()
{
	// Construct a new parser for each message
	_parser.emplace();

	// Apply a reasonable limit to the allowed size
	// of the body in bytes to prevent abuse.
	_parser->body_limit(10000);

	// Set the timeout.
	_stream.expires_after(std::chrono::seconds(30));

	// Read a request
	http::async_read(_stream, _buffer, _parser->get(),
			beast::bind_front_handler(&HttpSession::onRead, shared_from_this()));
}

void HttpSession::onRead(beast::error_code ec, std::size_t)
{
	// This means they closed the connection
	if(ec == http::error::end_of_stream)
	{
		_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
		return;
	}

	// Handle the error, if any
	if(ec)
	{
		return fail(ec, "read");
	}

	// See if it is a WebSocket Upgrade
//	if(websocket::is_upgrade(_parser->get()))
//	{
//		auto &&http_req = _parser->release();
////		std::cout << "###http_req:" << http_req.base().target() << std::endl;
//
//		// Respond to GET request
//
//		if(http_req.base().target().find(HTTP_KEY_INFO) == beast::string_view::npos) // as auth pass
//		{
////			http::response<http::string_body> res
////			{
////				http::status::unauthorized,
////				http_req.version()
////			};
////			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
////			res.set(http::field::content_type, "text/html");
////			res.keep_alive(http_req.keep_alive());
////			res.body() = std::string("#########3 auth failed");
////			res.prepare_payload();
////			auto const &&send = sendLambda(*this);
////			send(std::move(res));
//			handleRequest(http_req, sendLambda(*this));
//			return;
//		}
//
//		// Create a websocket session, transferring ownership
//		// of both the socket and the HTTP request.
//		std::make_shared<WebsocketSession>(_stream.release_socket())->run(http_req);
//		return;
//	}

	// Send the response
	//
	// This code uses the function object type sendLambda in
	// place of a generic lambda which is not available in C++11
	//
	handleRequest(_parser->release(), sendLambda(*this));

}

void HttpSession::onWrite(beast::error_code ec, std::size_t, bool close)
{
	if(ec)
	{
		return fail(ec, "write");
	}

	if(close)
	{
		// This means we should close the connection, usually because
		// the response indicated the "Connection: close" semantic.
		_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
		return;
	}

	// Read another request
	doRead();
}

