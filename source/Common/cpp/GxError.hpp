/*
 * GxError.hpp
 *
 *  Created on: Jan 11, 2020
 *      Author: wenhe
 */

#ifndef EXCEPTION_GXERROR_HPP_
#define EXCEPTION_GXERROR_HPP_

#include <string>
#include <sstream>
#include "RemoteCode.h"

template<typename T>
class GxError: public std::exception
{

public:

    GxError(const T code, const std::string& message) :
            _code(code), _message(message)
    {
    }
    virtual const char* what() const _GLIBCXX_USE_NOEXCEPT
    {
        return _message.c_str();
    }
    const T code()
    {
        return _code;
    }

private:
    T _code;
    std::string _message;

};

//using GxIntError = GxError<int>;
//using GxRcError = GxError<RC::Code>;

//#define throwGxIntError(code, message)\
//    do\
//    {\
//        std::stringstream ss;\
//        ss << __FILE__ << ":" << __LINE__ << " " << message;\
//        throw GxIntError(code, ss.str());\
//    }\
//while(0)
//
//#define throwGxRcError(code, message)\
//    do\
//    {\
//        std::stringstream ss;\
//        ss << __FILE__ << ":" << __LINE__ << " " << message;\
//        throw GxRcError(code, ss.str());\
//    }\
//while(0)







#endif /* EXCEPTION_GXERROR_HPP_ */
