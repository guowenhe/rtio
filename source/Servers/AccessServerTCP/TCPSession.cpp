/*
 * TCPSession.cpp
 *
 *  Created on: 24 Apr 2020
 *      Author: wenhe
 */


#include "IceClient.h"
#include "SessionManager.h"
#include "TCPSession.h"
#include "RtioLog.h"
#include "RemoteCode.h"


inline Ice::LoggerOutputBase& operator<<(Ice::LoggerOutputBase& out, const DMS::ClientStatus status) // todo temporary
{
    switch(status)
    {
    case DMS::ClientStatus::UNKNOWN:
        out << "UNKNOWN";
        break;
    case DMS::ClientStatus::ONLINE:
        out << "ONLINE";
        break;
    case DMS::ClientStatus::OFFLINE:
        out << "OFFLINE";
        break;
    default:
        out << "no-desc(" << static_cast<int>(status) << ")";
    }

    return out;
}

TCPSession::TCPSession(tcp::socket socket):
    _socket(std::move(socket)), _circleTimer(_socket.get_executor())
{
    auto *iceClient = GlobalResources::IceClient::getInstance();
    log2D(iceClient->getCommunicator(), "-");
    _messageId = ((uint64_t)(this) >> 16) & 0xFFFF;
}
TCPSession::~TCPSession()
{
    auto *iceClient = GlobalResources::IceClient::getInstance();
    log2D(iceClient->getCommunicator(), "-");
    GlobalResources::SessionManager::getInstance()->removeSession(_deviceId);
}

void TCPSession::run()
{
    receiveAuthHeader();
}
void TCPSession::receiveAuthHeader()
{
    asio::async_read(_socket, asio::buffer(_receiveMessage.header(), _receiveMessage.headerLength()),
            std::bind(&TCPSession::onReceiveAuthHeader, shared_from_this(), std::placeholders::_1));
}
void TCPSession::onReceiveAuthHeader(sys::error_code ec)
{
    if(ec)
    {
        return fail(ec, "onReceiveAuthHeader");
    }
    if(_receiveMessage.decodeHeader() != DProto::RetCode::SUCCESS)
    {
        return fail(ec, "onReceiveAuthHeader headerDecode error");
    }
    if(_receiveMessage.type() != DProto::Type::AUTH_REQ)
    {
        return fail(ec, "onReceiveAuthHeader type error");
    }

    receiveAuthBody();
}
void TCPSession::receiveAuthBody()
{
    asio::async_read(_socket, asio::buffer(_receiveMessage.body(), _receiveMessage.bodyLength()),
            std::bind(&TCPSession::onReceiveAuthBody, shared_from_this(), std::placeholders::_1));
}
void TCPSession::onReceiveAuthBody(sys::error_code ec)
{
    if(ec)
    {
        sendResponse(DProto::RemoteCode::SERVER_FAIL);
        return fail(ec, "onReceiveAuthBody SERVER_FAIL");
    }
    if(_receiveMessage.bodyLength() != DEVICE_AUTH_DATA_LENGTH)
    {
        auto iceClient = GlobalResources::IceClient::getInstance();
        log2I(iceClient->getCommunicator(), "auth data invalid");
        return sendResponse(DProto::RemoteCode::AUTH_DATA_INVALID);
    }
    _deviceId = std::string((char*)_receiveMessage.body(), DEVICE_AUTH_ID_LENGTH);
    _deviceKey = std::string((char*)(_receiveMessage.body() + DEVICE_AUTH_ID_LENGTH + 1), DEVICE_AUTH_KEY_LENGTH);

    authDevice();
}

void TCPSession::authDevice()
{
    GlobalResources::IceClient::getInstance()->authDevice(_deviceId, _deviceKey,
            std::bind(&TCPSession::onAuthDevice, shared_from_this(), std::placeholders::_1));

}
void TCPSession::onAuthDevice(GlobalResources::RetCode ret)
{
    auto iceClient = GlobalResources::IceClient::getInstance();
    log2I(iceClient->getCommunicator(), "Auth deviceId=" << _deviceId << " ret=" << ret);

    if(ret == GlobalResources::RetCode::AuthPass)
    {
        sendResponse(DProto::RemoteCode::AUTH_PASS);

        _deviceStatus = DMS::ClientStatus::ONLINE;
        setStatus();
    }
    else if(ret == GlobalResources::RetCode::AuthFail)
    {
        sendResponse(DProto::RemoteCode::AUTH_FAIL);
    }
    else if(ret == GlobalResources::RetCode::AuthIdInvalid || ret == GlobalResources::RetCode::AuthKeyInvalid)
    {
        sendResponse(DProto::RemoteCode::AUTH_DATA_INVALID);
    }
    else
    {
        sendResponse(DProto::RemoteCode::SERVER_FAIL);
    }
}
void TCPSession::sendResponse(DProto::RemoteCode code)
{
    DProto::Type type = (DProto::Type::AUTH_REQ == _receiveMessage.type())?     DProto::Type::AUTH_RESP:
                        (DProto::Type::REPORT_REQ == _receiveMessage.type())?   DProto::Type::REPORT_RESP:
                                                                                DProto::Type::UNSET;

    auto message = std::make_shared<DProto::Message>();
    message->setId(_receiveMessage.id());
    message->setType(type);
    message->setVersion(_receiveMessage.version());
    message->assembleBody_RespCode(code);
    message->encodeHeader();
    send(message);
}

void TCPSession::setStatus()
{
    GlobalResources::IceClient::getInstance()->setStatus(_deviceId, _deviceStatus,
            std::bind(&TCPSession::onSetStatus, shared_from_this(), std::placeholders::_1));
}


void TCPSession::onSetStatus(GlobalResources::RetCode ret)
{
    auto *iceClient = GlobalResources::IceClient::getInstance();
    if(GlobalResources::RetCode::Success == ret)
    {
        log2I(iceClient->getCommunicator(), "success, status=" << _deviceStatus);
        if(_deviceStatus == DMS::ClientStatus::ONLINE)
        {
            GlobalResources::SessionManager::getInstance()->addSession(_deviceId, this);
            doReadHeader();
            startCircleTimer();
        }
    }
    else
    {
        log2E(iceClient->getCommunicator(), "fail");
    }
}

void TCPSession::doReadHeader()
{
    asio::async_read(_socket, asio::buffer(_receiveMessage.header(), _receiveMessage.headerLength()),
            std::bind(&TCPSession::onReadHeader, shared_from_this(), std::placeholders::_1));
}
void TCPSession::onReadHeader(sys::error_code ec)
{
    if(ec)
    {
        return fail(ec, "onReadHeader");
    }

    if(_receiveMessage.decodeHeader() != DProto::RetCode::SUCCESS)
    {
        return fail(ec, "headerDecode error");
    }
    if(_receiveMessage.type() != DProto::Type::REPORT_REQ &&
            _receiveMessage.type() != DProto::Type::DISPATCH_RESP)
    {
        sendResponse(DProto::RemoteCode::TYPE_ERROR);
        return fail(ec, "headerDecode type error");
    }

    doReadBody();
}
void TCPSession::doReadBody()
{
    asio::async_read(_socket, asio::buffer(_receiveMessage.body(), _receiveMessage.bodyLength()),
            std::bind(&TCPSession::onReadBody, shared_from_this(), std::placeholders::_1));
}

void TCPSession::onReadBody(sys::error_code ec)
{
    if(ec)
    {
        return fail(ec, "onReadHeader");
    }

    auto* iceClient = GlobalResources::IceClient::getInstance();

    try
    {
        switch(_receiveMessage.type())
        {
        case DProto::Type::REPORT_REQ:
        {
            sendResponse(DProto::RemoteCode::SUCCESS);
            GlobalResources::RetCode ret = GlobalResources::IceClient::getInstance()->reportMessage(
                    std::string((char*) _receiveMessage.body(), _receiveMessage.bodyLength()), _deviceId);
            if(GlobalResources::RetCode::Success != ret)
            {
                log2E(iceClient->getCommunicator(), "reportMessage error, ret=" << ret);
            }
            break;
        }
        case DProto::Type::DISPATCH_RESP:
        {
            log2D(iceClient->getCommunicator(), "call response for ice dispatch");

            auto data = std::make_shared<DispatchRespData>();
            DProto::RemoteCode code = _receiveMessage.getFromBody_RespCode();
            if(DProto::RemoteCode::SUCCESS == code)
            {
                data->code = RC::Code::SUCCESS;
                if(_receiveMessage.getFromBody_RespDataLength())
                {
                    data->deviceMessage.assign((char*)_receiveMessage.getFromBody_RespData(),
                            (char*)(_receiveMessage.getFromBody_RespData() + _receiveMessage.getFromBody_RespDataLength()));
                }
            }
            else
            {
                data->code = RC::Code::FAIL;
                log2E(iceClient->getCommunicator(),
                        "dispathc to device error, code=" << std::hex << static_cast<unsigned>(code));
            }

            _responseSession.response(_receiveMessage.id(), data);
            break;
        }
        default:
            log2E(iceClient->getCommunicator(),
                    "onReadBody type error" << std::hex << static_cast<unsigned>(_receiveMessage.type()));
            break;
        }
    }
    catch(std::exception& ex)
    {
        log2E(iceClient->getCommunicator(), "exception=" << ex.what());
    }

    doReadHeader();
}


void TCPSession::fail(sys::error_code ec, char const *what)
{
    auto *iceClient = GlobalResources::IceClient::getInstance();

    stopCircleTimer();

    if(DMS::ClientStatus::ONLINE == _deviceStatus)
    {
        _deviceStatus = DMS::ClientStatus::OFFLINE;
        setStatus();
    }

    if(ec == asio::error::operation_aborted)
    {
        log2D(iceClient->getCommunicator(), what << ": " << ec.message());
        return;
    }
    log2E(iceClient->getCommunicator(), what << ": " << ec.message());
}

void TCPSession::dispatch(const std::string& message, const ResponseFun& respone)
{
    auto *iceClient = GlobalResources::IceClient::getInstance();

    auto m = std::make_shared<DProto::Message>();
    m->setId(genMessageId());
    m->setVersion(m->currentVersion());
    m->setType(DProto::Type::DISPATCH_REQ);
    m->assembleBody_ReqData((uint8_t *)message.c_str(), message.length());
    m->encodeHeader();

    int ret = _responseSession.monitor(m->id(), respone);
    if(ret < 0)
    {
        log2E(iceClient->getCommunicator(), "responseSession.monitor error");
    }
    send(m);
}

void TCPSession::send(std::shared_ptr<DProto::Message> const& message)
{
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.

    asio::post(_socket.get_executor(), std::bind(&TCPSession::onSend, shared_from_this(), message));
}

void TCPSession::onSend(std::shared_ptr<DProto::Message> const& message)
{
    _sendMessageQueue.push_back(message);

    // Are we already writing?
    if(_sendMessageQueue.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    auto m = _sendMessageQueue.front();
    asio::async_write(_socket, asio::buffer(m->header(), m->transLength()),
            std::bind(&TCPSession::onWrite, shared_from_this(), std::placeholders::_1));
}

void TCPSession::onWrite(sys::error_code ec)
{
    if(ec)
    {
        return fail(ec, "write");
    }

    _sendMessageQueue.erase(_sendMessageQueue.begin());

    // Send the next message if any
    if(!_sendMessageQueue.empty())
    {
        auto m = _sendMessageQueue.front();
          asio::async_write(_socket, asio::buffer(m->header(), m->transLength()),
                  std::bind(&TCPSession::onWrite, shared_from_this(), std::placeholders::_1));

    }
}
void TCPSession::startCircleTimer()
{
    _circleTimer.expires_after(std::chrono::seconds(TCPSESSION_TIMER_CYCLE));
    _circleTimer.async_wait(std::bind(&TCPSession::onCircleTimer, shared_from_this()));
    _circleTimerRun = true;
}
void TCPSession::stopCircleTimer()
{
    _circleTimerRun = false;
    _circleTimer.cancel();
}
void TCPSession::onCircleTimer()
{
    auto *iceClient = GlobalResources::IceClient::getInstance();
    log2D(iceClient->getCommunicator(), "check.");

    auto data = std::make_shared<DispatchRespData>();
    data->code = RC::Code::ACCESSSERVER_TIMEOUT;
    _responseSession.responseTimeouts(data);
    if(_circleTimerRun)
    {
        _circleTimer.expires_after(std::chrono::seconds(TCPSESSION_TIMER_CYCLE));
        _circleTimer.async_wait(std::bind(&TCPSession::onCircleTimer, shared_from_this()));
    }

}
time_t TCPSession::getClock()
{
    auto *iceClient = GlobalResources::IceClient::getInstance();
    struct timespec t;
    int ret = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    if(ret)
    {
        log2E(iceClient->getCommunicator(), "clock_gettime error, ret=" << ret << " error=" << errno);
        return -1;
    }
    log2D(iceClient->getCommunicator(), "t.tv_sec=" << t.tv_sec);
    return  t.tv_sec;
}


