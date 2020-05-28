/*
 * APINotifierI.h
 *
 *  Created on: 29 Feb 2020
 *      Author: wenhe
 */

#ifndef APINOTIFIERI_H_
#define APINOTIFIERI_H_

#include "HttpSession.h"
#include "APINotifier.h"
namespace DMS
{

class APINotifierI: public virtual APINotifier
{
public:
    APINotifierI(asio::io_context& ioc):
        _ioc(ioc)
    {
    }
    virtual void notifyAsync(::std::shared_ptr<NotifyReq> req,
            ::std::function<void(const ::std::shared_ptr<NotifyResp>& resp)> response,
            ::std::function<void(::std::exception_ptr)> exception, const ::Ice::Current& current);
private:
    asio::io_context& _ioc;

};


} //DMS



#endif /* APINOTIFIERI_H_ */
