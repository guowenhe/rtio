/*
 * WebsocketSession.h
 *
 *  Created on: Dec 8, 2019
 *      Author: szz
 */

#ifndef WEBSOCKETSESSION_H_
#define WEBSOCKETSESSION_H_

#include <memory>
#include "Beast.h"
#include "ResponseManager.hpp"
#include "DeviceHub.h"

#define WEBSOCKETSESSION_TIMER_CYCLE 5
using ResponseType = int;
using ResponseFun = ::std::function<void(ResponseType)>;

class WebsocketSession: public std::enable_shared_from_this<WebsocketSession>
{
public:

	WebsocketSession(tcp::socket&& socket, const std::string &deviceId);
	~WebsocketSession();

	template<class Body, class Allocator>
	void run(http::request<Body, http::basic_fields<Allocator>> req);

	void dispatch(const std::string& message, const ResponseFun& respone);
	void send(std::shared_ptr<std::string const> const& message);

private:
	void fail(beast::error_code ec, char const *what);
	void onAccept(beast::error_code ec);
	void read();
	void onRead(beast::error_code ec, std::size_t bytes_transferred);
	void onWrite(beast::error_code ec, std::size_t bytes_transferred);
	void onSend(std::shared_ptr<std::string const> const& message);
	void onTimer(beast::error_code ec);
	time_t getClock();
	int genMessageId() {return ((_messageId++) & 0xFFFF);}
	void setStatus();
	void onSetStatus(int code);

	beast::flat_buffer _buffer;
	websocket::stream<beast::tcp_stream> _ws;
	std::string _deviceId;
	std::vector<std::shared_ptr<std::string const>> _queue;
	asio::steady_timer _timer;
	Util::ResponseManager<ResponseFun, ResponseType> _responseSession;
	int _messageId = 0;
	DMS::ClientStatus _targetStatus = DMS::ClientStatus::ONLINE;
};

template<class Body, class Allocator>
void WebsocketSession::run(http::request<Body, http::basic_fields<Allocator>> req)
{
	// Set suggested timeout settings for the websocket
	_ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

	// Set a decorator to change the Server of the handshake
	_ws.set_option(websocket::stream_base::decorator([](websocket::response_type &res)
	{
		res.set(http::field::server,
				std::string(BOOST_BEAST_VERSION_STRING) +
				" websocket-chat-multi");
	}));

	// Accept the websocket handshake
	_ws.async_accept(req, beast::bind_front_handler(&WebsocketSession::onAccept, shared_from_this()));
}





#endif /* WEBSOCKETSESSION_H_ */
