/*
 * VerijsLog.h
 *
 *  Created on: Dec 7, 2019
 *      Author: szz
 */

#ifndef VERIJSLOG_H_
#define VERIJSLOG_H_

#include <Ice/Ice.h>
#include <thread>
#include "Common.h"

#define _LOG_PREFIX         IceUtil::Time::now().toDateTime() << "|" << std::this_thread::get_id() << "|" << Util::getFileName(__FILE__) << ":" << __LINE__  << "|" << __FUNCTION__ << "|"
#define _LOG_PREFIX_SN(sn)  IceUtil::Time::now().toDateTime() << "|" << std::this_thread::get_id() << "|" << Util::getFileName(__FILE__) << ":" << __LINE__  << "|" << sn << "|" << __FUNCTION__ << "|"

#define _LOG_SN(content, level)\
    do\
    {\
        if(nullptr != __c)\
        {\
            Ice::Print out(__c->getLogger());\
            out << #level"|" << _LOG_PREFIX_SN(__sn) << content;\
        }\
    }\
    while(0)


#define _LOG(communicator, content, level)\
    do\
    {\
        if(nullptr != communicator)\
        {\
            Ice::Print out(communicator->getLogger());\
            out << #level"|" << _LOG_PREFIX << content;\
        }\
    }\
    while(0)

// no sn (serial number)
#define log2E(communicator, content) _LOG(communicator, content, E)
#define log2W(communicator, content) _LOG(communicator, content, W)
#define log2I(communicator, content) _LOG(communicator, content, I)
#define log2D(communicator, content) _LOG(communicator, content, D)

// with sn (serial number), required call logSet first in function
#define logSet(communicator, sn) const ::Ice::Communicator* __c = communicator.get(); const int __sn = sn;
#define logE(content) _LOG_SN(content, E)
#define logW(content) _LOG_SN(content, W)
#define logI(content) _LOG_SN(content, I)
#define logD(content) _LOG_SN(content, D)






#endif /* VERIJSLOG_H_ */
