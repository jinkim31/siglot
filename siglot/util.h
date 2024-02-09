#ifndef SIGLOT_UTIL_H
#define SIGLOT_UTIL_H

#include <string>
#include <iostream>

static std::string functionNameWithNamespaceToSiglotId(const std::string& name)
{
    // input check
    if(name.empty())
        return "";

    // find last colon(from namespace operator ::)
    int startIdx;
    for(startIdx = name.size()-1; 0 <= startIdx; startIdx--)
        if (name[startIdx] == ':')
            break;

    // check colon exists
    if(startIdx == 0)
        return "";

    return {&name[startIdx+1]};
}

#endif