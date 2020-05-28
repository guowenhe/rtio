/*
 * Base64.h
 *
 *  Created on: 9 Apr 2020
 *      Author: wenhe
 */

#include <boost/beast/core/detail/base64.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <map>
namespace Util
{

std::string base64Encode(std::string_view s)
{
    // implement refer boost source (beast/test/beast/core/_detail_base64.cpp)
    namespace base64 = boost::beast::detail::base64;
    std::string dest;
    dest.resize(base64::encoded_size(s.size()));
    dest.resize(base64::encode(&dest[0], s.data(), s.size()));
    return dest;
}
std::string base64Decode(std::string_view s)
{
    // implement refer boost source (beast/test/beast/core/_detail_base64.cpp)
    namespace base64 = boost::beast::detail::base64;
    std::string dest;
    dest.resize(base64::decoded_size(s.size()));
    auto const r = base64::decode(&dest[0], s.data(), s.size());
    dest.resize(r.first);
    return dest;
}

void urlQueryStringParse(const std::string_view& queryString, std::map<std::string, std::string>& queryMap)
{
    std::vector<std::string> v1;
    boost::split(v1, queryString, boost::is_any_of("&"), boost::token_compress_on);

    for(auto& c: v1)
    {
        std::vector<std::string> v2;
        boost::split(v2, c, boost::is_any_of("="), boost::token_compress_on);
        if(v2.size() == 2)
        {
            queryMap[v2[0]] = v2[1];

        }
        else
        {
            queryMap[v2[0]] = "";
        }
    }
}
void urlQueryStringSerial(const std::map<std::string, std::string>& queryMap, std::string& queryString)
{
    for(auto& c: queryMap)
    {
        queryString += c.first + "=" + c.second + "&";
    }
    queryString.resize(queryString.size() - 1);
}

const static std::array<char, 16> __HexTable{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

std::string binToHex(std::string_view a)
{
    std::string h;
    h.resize(a.size() * 2);
    auto it = h.begin();
    for(auto& c : a)
    {
        *it++ = __HexTable[(c >> 4) & 0x0F];
        *it++ = __HexTable[c & 0x0F];
    }
    return h;
}

} // Util




