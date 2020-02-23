/*
 * Listener.cpp
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */


#include "Listener.h"

#include "VerijsLog.h"

#include "HttpSession.h"
#include "IceClient.h"

void Listener::fail(beast::error_code ec, char const *what)
{
	// Don't report on canceled operations
	if(ec == asio::error::operation_aborted)
	{
		return;
	}
	auto *iceClient = GlobalResources::IceClient::getInstance();
	log2E(iceClient->getCommunicator(), what << ": " << ec.message());
}

Listener::Listener(asio::io_context &ioc, tcp::endpoint endpoint) :
		_ioc(ioc), _acceptor(ioc)
{
	beast::error_code ec;

	_acceptor.open(endpoint.protocol(), ec);
	if(ec)
	{
		fail(ec, "open");
		return;
	}

	_acceptor.set_option(asio::socket_base::reuse_address(true), ec);
	if(ec)
	{
		fail(ec, "set_option");
		return;
	}

	_acceptor.bind(endpoint, ec);
	if(ec)
	{
		fail(ec, "bind");
		return;
	}

	_acceptor.listen(asio::socket_base::max_listen_connections, ec);
	if(ec)
	{
		fail(ec, "listen");
		return;
	}
}

void Listener::run()
{
	_acceptor.async_accept(asio::make_strand(_ioc), beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
}

// Handle a connection
void Listener::onAccept(beast::error_code ec, tcp::socket socket)
{
	if(ec)
	{
		return fail(ec, "accept");
	}
	else
	{
		// Launch a new session for this connection
		std::make_shared<HttpSession>(std::move(socket))->run();
	}

	_acceptor.async_accept(asio::make_strand(_ioc), beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
}

