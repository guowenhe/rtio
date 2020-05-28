/*
 * HttpSession.cpp
 *
 *  Created on: Dec 9, 2019
 *      Author: szz
 */
#include "HttpSession.h"
#include "RtioLog.h"
#include "Json.h"
#include "Common.h"
#include "RtioExcept.h"

void HttpSession::responseEx(std::shared_ptr<HttpRespInfo> info, std::shared_ptr<GlobalResources::CreateDeviceReturn> data)
{
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.
    asio::post(_stream.get_executor(),
            beast::bind_front_handler(&HttpSession::response, shared_from_this(), info, data));
}

void HttpSession::response(std::shared_ptr<HttpRespInfo> info, std::shared_ptr<GlobalResources::CreateDeviceReturn> data)
{
    http::status status;
    if(data)
    {
        status = (RC::Code::SUCCESS == data->code || RC::Code::API_SENDER_DEVICE_NOTFOUND == data->code) ?
                        http::status::ok : http::status::internal_server_error;
    }
    else
    {
        status = info->status;
    }

    auto resp = std::make_shared<http::response<http::string_body>>(status, info->version);
    resp->set(http::field::server, RTIO_SERVER_STRING);
    resp->set(http::field::content_type, "text/html");
    resp->keep_alive(info->keepAlive);

    if(http::status::ok == status)
    {
        if(data)
        {
            Util::json j;
            j["nonce"] = info->para.nonce;
            j["code"] = RC::codeToInt(data->code);
            j["desc"] = RC::describe(data->code);
            j["device_id"] = data->deviceId;
            j["device_key"] = data->deviceKey;
            resp->body() = j.dump();
        }
    }
    else
    {
        resp->body() = info->body;
    }
    resp->prepare_payload();

    // Write the response, need ref resp pointer
//    http::async_write(_stream, *resp,
//            beast::bind_front_handler(&HttpSession::onWrite, shared_from_this(), resp->need_eof()));
    http::async_write(_stream, *resp, [self = shared_from_this(), resp](beast::error_code ec, std::size_t bytes)
    {
        self->onWrite(resp->need_eof(), ec, bytes);
    });
}


template<class Body, class Allocator>
void HttpSession::handleRequest(http::request<Body, http::basic_fields<Allocator>>&& req)
{
    auto info = std::make_shared<HttpRespInfo>();
    info->version = req.version();
    info->keepAlive = req.keep_alive();

    auto handleError = [this, &info](RC::Code code, const std::string& message)
    {
        auto *iceClient = GlobalResources::IceClient::getInstance();
        log2E(iceClient->getCommunicator(), "code="<< code << "message=" << message);
        Util::json j;
        if(!info->para.nonce.empty())
        {
            j["nonce"] = info->para.nonce;
        }
        j["code"] = RC::codeToInt(code);
        j["desc"] = message;
        info->body = j.dump();
        info->status = http::status::bad_request;
        response(info);
    };

    auto checkOptionInt= [](Util::json& j, const std::string& field)
    {
        if(j[field].is_number_integer())
        {
            return std::pair<bool, int>(true, j[field]);
        }
        return std::pair<bool, int>(false, 0);
    };
    auto checkReqirdString = [](Util::json& j, const std::string& field, size_t size)
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

    try
    {
        if(req.method() != http::verb::post)
        {
            throw RC::Except(RC::Code::API_MANAGE_METHOD_ERROR);
        }

        if(req.target().size() != uriSend.size() || req.target() != uriSend)
        {
            throw RC::Except(RC::Code::API_MANAGE_URI_INVALID);
        }

        if(req.body().size() > 3072)
        {
            throw RC::Except(RC::Code::API_MANAGE_BODY_INVALID);
        }

        int timeLive = 10; // default
        auto reqJson = Util::json::parse((std::string)req.body());
        info->para.nonce = checkReqirdString(reqJson, "nonce", 32);

        std::function<void(std::shared_ptr<GlobalResources::CreateDeviceReturn>)> cb =
                 std::bind(&HttpSession::responseEx, shared_from_this(), info, std::placeholders::_1);

        auto ret = GlobalResources::IceClient::getInstance()->createDevice(info->para, cb);
        if(GlobalResources::RetCode::Success != ret)
        {
            log2E(GlobalResources::IceClient::getInstance()->getCommunicator(), "dispatch error, ret=" << ret);
            info->status = http::status::internal_server_error;

            throw RC::Except(RC::Code::API_SENDER_DISPATCH_FAIL);
        }
    }
    catch(Util::json::parse_error& ex)
    {
        handleError(RC::Code::API_SENDER_JSON_PARSE_FAIL, ex.what());
    }
    catch(RC::Except& ex)
    {
        handleError(ex.code(), ex.what());
    }
    catch(std::exception& ex)
    {
        handleError(RC::Code::FAIL, ex.what());
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
    std::cout << "#" << __FUNCTION__ << std::endl;
}

HttpSession::~HttpSession()
{
    std::cout << "#" << __FUNCTION__ << std::endl;
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

	handleRequest(_parser->release());

    //
    // The following code requires generic
    // lambdas, available in C++14 and later.
    //
//	doRequest(_parser->release(),
//        [this](auto&& response)
//        {
//            // The lifetime of the message has to extend
//            // for the duration of the async operation so
//            // we use a shared_ptr to manage it.
//            using response_type = typename std::decay<decltype(response)>::type;
//            auto sp = boost::make_shared<response_type>(std::forward<decltype(response)>(response));
//
//            // NOTE This causes an ICE in gcc 7.3
//            // Write the response
//            http::async_write(this->_stream, *sp,
//                    [self = shared_from_this(), sp](
//                            beast::error_code ec, std::size_t bytes)
//                    {
////                        self->onWrite(ec, bytes, sp->need_eof());
//                    });
//        });

}

void HttpSession::onWrite(bool close, beast::error_code ec, std::size_t)
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

