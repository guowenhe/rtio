/*
 * RtioExcept.h
 *
 *  Created on: 22 Feb 2020
 *      Author: wenhe
 */

#ifndef EXCEPTION_RTIOEXCEPT_H_
#define EXCEPTION_RTIOEXCEPT_H_

#include <string>
#include "Common.h"
namespace Rtio
{

template<typename T>
class Except: public std::exception
{
public:
    Except(const T code, const std::string& message) :
            _code(code), _message(message)
    {
    }
    Except(const T code, const std::string& message, const std::string& where) :
            _code(code), _message(where + message)
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
    const std::string _message;
};



std::string inline _where(const char* file, const int line, const char* fun)
{
    std::string s;
    s += "(";
    s += Util::getFileName(file);
    s += ":";
    s += std::to_string(line);
    s += ":";
    s += fun;
    s += ")";
    return s;
}

#define Rtio_where() Rtio::_where(__FILE__, __LINE__, __FUNCTION__)




} //Rtio



#endif /* EXCEPTION_RTIOEXCEPT_H_ */
