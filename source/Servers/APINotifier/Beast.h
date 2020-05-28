/*
 * Beast.h
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */

#ifndef BEAST_H_
#define BEAST_H_

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>


namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;



#endif /* BEAST_H_ */
