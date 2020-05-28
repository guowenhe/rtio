/*
 * HttpSession.cpp
 *
 *  Created on: 1 Mar 2020
 *      Author: wenhe
 */

#include "HttpSession.h"
#include "ServerGlobal.h"
#include "RemoteCode.h"
#include "RtioLog.h"
#include "Json.h"
#include "Common.h"


HttpClientSession::HttpClientSession(asio::io_context& ioc) :
        _resolver(asio::make_strand(ioc)), _stream(asio::make_strand(ioc))
{
}

// Start the asynchronous operation
void HttpClientSession::run(std::shared_ptr<HttpClientReq>& clientReq,
        std::function<void(std::shared_ptr<HttpClientResp>)> clientResponse)
{
    _sn = clientReq->sn;
    logSet(ServerGlobal::getInstance()->getCommunicator(), _sn);
    logI("url=" << clientReq->host << ":" << clientReq->port << clientReq->target);

    _clientResponse = clientResponse;

    // Set up an HTTP POST request message
    _req.version(11);
    _req.method(http::verb::post);
    _req.target(clientReq->target);
    _req.set(http::field::host, clientReq->host);
    _req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    auto nonce = std::to_string(Util::IdGenerator::getInstance()->get());
    logI("http req nonce=" << nonce);

    Util::json j;
    j["nonce"] = nonce;
    j["device_id"] = clientReq->deviceId;
    j["device_message"] = clientReq->message;
    _req.body() = j.dump();
    _req.prepare_payload();
    logD("http req body=" << _req.body());

    // Look up the domain name
    _resolver.async_resolve(clientReq->host, clientReq->port,
            beast::bind_front_handler(&HttpClientSession::onResolve, shared_from_this()));
}

void HttpClientSession::onResolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if(ec)
    {
        return fail(ec, "resolve");
    }

    // Set a timeout on the operation
    _stream.expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    _stream.async_connect(results, beast::bind_front_handler(&HttpClientSession::onConnect, shared_from_this()));
}

void HttpClientSession::onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if(ec)
    {
        return fail(ec, "connect");
    }

    // Set a timeout on the operation
    _stream.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(_stream, _req, beast::bind_front_handler(&HttpClientSession::onWrite, shared_from_this()));
}

void HttpClientSession::onWrite(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        return fail(ec, "write");
    }

    // Set a timeout on the operation
    _stream.expires_after(std::chrono::seconds(30));

    // Receive the HTTP response
    http::async_read(_stream, _buffer, _resp, beast::bind_front_handler(&HttpClientSession::onRead, shared_from_this()));
}

void HttpClientSession::onRead(beast::error_code ec, std::size_t bytes_transferred)
{
    logSet(ServerGlobal::getInstance()->getCommunicator(), _sn);
    logI("-");
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        return fail(ec, "read");
    }

    auto checkOptionInt= [](Util::json& j, const std::string& field)
    {
        if(j[field].is_number_integer())
        {
            return std::pair<bool, int>(true, j[field]);
        }
        return std::pair<bool, int>(false, 0);
    };
    auto checkReqiredInt= [](Util::json& j, const std::string& field)
    {
        if(!j[field].is_number_integer())
        {
            std::string errorMessage(RC::describe(RC::Code::API_SENDER_JSON_FIELD_INVALID));
            errorMessage += ":";
            errorMessage += field;
            throw RC::Except(RC::Code::API_SENDER_JSON_FIELD_INVALID, errorMessage);
        }
        return j[field];
    };
    auto checkReqiredString = [](Util::json& j, const std::string& field, size_t size)
    {
        if(!j[field].is_string())
        {
            std::string errorMessage(RC::describe(RC::Code::API_SENDER_JSON_FIELD_INVALID));
            errorMessage += ":";
            errorMessage += field;
            throw RC::Except(RC::Code::API_SENDER_JSON_FIELD_INVALID, errorMessage);
        }
        if(static_cast<std::string>(j[field]).size() > size)
        {
            std::string errorMessage(RC::describe(RC::Code::API_SENDER_JSON_FIELD_INVALID));
            errorMessage += ":";
            errorMessage += field;
            errorMessage += ",exceed limit:";
            throw RC::Except(RC::Code::API_SENDER_JSON_FIELD_INVALID, errorMessage);
        }

        return j[field];
    };

    auto resp = std::make_shared<HttpClientResp>();
    resp->sn = _sn;
    try
    {
        logD("http resp body=" << _resp.body().substr(0, 128));
        auto respJson = Util::json::parse((std::string)_resp.body());
        std::string nonce = checkReqiredString(respJson, "nonce", 32);
        logD("http resp nonce=" << nonce);
        resp->code = checkReqiredInt(respJson, "code");
    }
    catch(std::exception& ex)
    {
        logE("http resp body=" << _resp.body().substr(0, 128));
        resp->code = RC::codeToInt(RC::Code::FAIL);

    }
    _clientResponse(resp);

    // Gracefully close the socket
    _stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if(ec && ec != beast::errc::not_connected)
    {
        return fail(ec, "shutdown");
    }

    // If we get here then the connection is closed gracefully
}

// Report a failure
void HttpClientSession::fail(beast::error_code ec, char const* what)
{
    logSet(ServerGlobal::getInstance()->getCommunicator(), _sn);
    logE(what << ": " << ec.message());

    auto resp = std::make_shared<HttpClientResp>();
    resp->sn = _sn;

    if(ec == beast::error::timeout)
    {
        resp->code = RC::codeToInt(RC::Code::API_NOTIFIER_TIMEOUT);
    }
    else
    {
        resp->code = RC::codeToInt(RC::Code::FAIL);
    }
    _clientResponse(resp);
}

