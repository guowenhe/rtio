/*
 * Listener.cpp
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */

//#include <IceClient.h>
#include "Listener.h"

#include "RtioLog.h"
#include "TCPSession.h"


void Listener::fail(sys::error_code ec, char const *what)
{
	// Don't report on canceled operations
	if(ec == asio::error::operation_aborted)
	{
		return;
	}
//	auto *iceClient = GlobalResources::IceClient::getInstance();
//	log2E(iceClient->getCommunicator(), what << ": " << ec.message());
}

Listener::Listener(asio::io_context &ioc, tcp::endpoint endpoint) :
		_ioc(ioc), _acceptor(ioc)
{
	sys::error_code ec;

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
    _acceptor.async_accept(asio::make_strand(_ioc),
            std::bind(&Listener::onAccept, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

// Handle a connection
void Listener::onAccept(sys::error_code ec, tcp::socket socket)
{
	if(ec)
	{
		return fail(ec, "accept");
	}
	else
	{
		// Launch a new session for this connection
		std::make_shared<TCPSession>(std::move(socket))->run();
	}

    _acceptor.async_accept(asio::make_strand(_ioc),
            std::bind(&Listener::onAccept, shared_from_this(), std::placeholders::_1, std::placeholders::_2));}

