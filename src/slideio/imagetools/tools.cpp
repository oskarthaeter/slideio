// This file is part of slideio project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://slideio.com/license.html.
//
#include "slideio/imagetools/tools.hpp"
#include <boost/algorithm/string.hpp>
#if defined(WIN32)
#include <Shlwapi.h>
#else
#include <fnmatch.h>
#endif
using namespace slideio;
extern "C" int wildmat(char* text, char* p);


bool Tools::matchPattern(const std::string& path, const std::string& pattern)
{
    bool ret(false);
#if defined(WIN32)
    ret = PathMatchSpecA(path.c_str(), pattern.c_str())!=0;
#else
    std::vector<std::string> subPatterns;
    boost::algorithm::split(subPatterns, pattern, boost::is_any_of(";"), boost::algorithm::token_compress_on);
    for(const auto& sub_pattern : subPatterns)
    {
        ret = wildmat(const_cast<char*>(path.c_str()),const_cast<char*>(sub_pattern.c_str()));
        if(ret){
            break;
        }
    }
#endif
    return ret;
}

inline void convertPair12BitsTo16Bits(uint8_t* source, uint16_t* target)
{
    target[0] = (*((uint16_t*)source)) & 0xFFF;
    uint8_t* next = source + 1;
    target[1] = (*((uint16_t*)(source+1))) >> 4;

}

void Tools::convert12BitsTo16Bits(uint8_t* source, uint16_t* target, int targetLen)
{
    uint16_t buff[2] = { 0 };
    uint16_t* targetPtr = target;
    uint8_t* sourcePtr = source;
    int srcBits = 0;
    for(int ind=0; ind<targetLen; ind+=2, targetPtr+=2, sourcePtr+=3)
    {
        if((ind+1)<targetLen)
        {
            convertPair12BitsTo16Bits(sourcePtr, targetPtr);
        }
        else
        {
            *targetPtr = (*((uint16_t*)sourcePtr)) & 0xFFF;
        }
    }
}


