/*
 * HttpSession.h
 *
 *  Created on: 1 Mar 2020
 *      Author: wenhe
 */

#ifndef HTTPSESSION_H_
#define HTTPSESSION_H_
#include "Beast.h"


struct HttpClientReq
{
    int sn;
    std::string host;
    std::string port;
    std::string target;
    std::string deviceId;
    std::string message;
};
struct HttpClientResp
{
    int sn;
    int code;
    std::string message;
};

// Performs an HTTP GET and prints the response
class HttpClientSession: public std::enable_shared_from_this<HttpClientSession>
{
public:
    // Objects are constructed with a strand to
    // ensure that handlers do not execute concurrently.
    explicit HttpClientSession(asio::io_context& ioc);

    // Start the asynchronous operation
    void run(std::shared_ptr<HttpClientReq>& clientReq,
            std::function<void(std::shared_ptr<HttpClientResp>)> clientResponse);
    void onResolve(beast::error_code ec, tcp::resolver::results_type results);
    void onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
    void onWrite(beast::error_code ec, std::size_t bytes_transferred);
    void onRead(beast::error_code ec, std::size_t bytes_transferred);
    void check_deadline();
    void fail(beast::error_code ec, char const* what);

private:
    tcp::resolver _resolver;
    beast::tcp_stream _stream;
    beast::flat_buffer _buffer; // (Must persist between reads)
    http::request<http::string_body> _req;
    http::response<http::string_body> _resp;
    int _sn = 0;
    std::function<void(std::shared_ptr<HttpClientResp>)> _clientResponse;
};


#endif /* HTTPSESSION_H_ */
