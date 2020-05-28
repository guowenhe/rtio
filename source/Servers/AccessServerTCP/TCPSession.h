/*
 * TCPSession.h
 *
 *  Created on: 24 Apr 2020
 *      Author: wenhe
 */

#ifndef TCPSESSION_H_
#define TCPSESSION_H_

#include <memory>
#include "Boost.h"
#include "ResponseManager.hpp"
#include "DeviceHub.h"
#include "CodecAdapter.hpp"
#include "RemoteCode.h"

#define TCPSESSION_TIMER_CYCLE 5

#define DEVICE_AUTH_ID_LENGTH   32
#define DEVICE_AUTH_KEY_LENGTH  27
#define DEVICE_AUTH_DATA_LENGTH (DEVICE_AUTH_ID_LENGTH+1+DEVICE_AUTH_KEY_LENGTH) //32+1+27
struct DispatchRespData
{
    RC::Code code = RC::Code::FAIL;
    std::string deviceMessage;
};

using ResponseType = std::shared_ptr<DispatchRespData>;
using ResponseFun = ::std::function<void(ResponseType)>;

class TCPSession : public std::enable_shared_from_this<TCPSession>
{
public:
    TCPSession(tcp::socket socket);
    ~TCPSession();

    void run();
    void send(std::shared_ptr<DProto::Message> const& message);
    void dispatch(const std::string& message, const ResponseFun& respone);

private:
    // device auth req
    void receiveAuthHeader();
    void onReceiveAuthHeader(sys::error_code ec);
    void receiveAuthBody();
    void onReceiveAuthBody(sys::error_code ec);
    // auth req internal
    void authDevice();
    void onAuthDevice(GlobalResources::RetCode ret);
    // device auth resp
    void sendResponse(DProto::RemoteCode code);
    // set status
    void setStatus();
    void onSetStatus(GlobalResources::RetCode ret);

    void doReadHeader();
    void onReadHeader(sys::error_code ec);
    void doReadBody();
    void onReadBody(sys::error_code ec);

    void onSend(std::shared_ptr<DProto::Message> const& message);
    void onWrite(sys::error_code ec);
    void startCircleTimer();
    void stopCircleTimer();
    void onCircleTimer();
    time_t getClock();
    uint16_t genMessageId() {return (_messageId++);}


    void fail(sys::error_code ec, char const *what);
    DProto::Message _receiveMessage;
    tcp::socket _socket;
    std::string _deviceId;
    std::string _deviceKey;
    std::vector<std::shared_ptr<DProto::Message>> _sendMessageQueue;
    asio::steady_timer _circleTimer;
    bool _circleTimerRun = false;
    Util::ResponseManager<ResponseFun, ResponseType> _responseSession;
    uint16_t _messageId = 0;
    DMS::ClientStatus _deviceStatus = DMS::ClientStatus::UNKNOWN;
};



#endif /* TCPSESSION_H_ */
