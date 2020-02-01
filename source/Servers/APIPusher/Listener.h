/*
 * Listener.h
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */

#ifndef LISTENER_H_
#define LISTENER_H_


#include "Beast.h"

#include <memory>
#include <string>


class Listener : public std::enable_shared_from_this<Listener>
{
public:
    Listener(asio::io_context& ioc, tcp::endpoint endpoint);
    void run();

private:
    void fail(beast::error_code ec, char const* what);
    void onAccept(beast::error_code ec, tcp::socket socket);

    asio::io_context& _ioc;
    tcp::acceptor _acceptor;
};



#endif /* LISTENER_H_ */
