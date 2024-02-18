#ifndef SIGLOT_UTIL_H
#define SIGLOT_UTIL_H

#include <string>
#include <iostream>

namespace siglot
{
static std::string functionNameWithNamespaceToSiglotId(const std::string &name)
{
    // input check
    if (name.empty())
        return "";

    // find last colon(from namespace operator ::)
    int startIdx;
    for (startIdx = name.size() - 1; 0 <= startIdx; startIdx--)
        if (name[startIdx] == ':')
            break;

    // check colon exists
    if (startIdx == 0)
        return "";

    return {&name[startIdx + 1]};
}

class PassProbe
{
public:
    explicit PassProbe(const int &id)
    {
        mId = id;
        std::cout << "PassTester(" << id << ") constructed" << std::endl;
    }

    PassProbe(const PassProbe &passTester)
    {
        mId = passTester.mId;
        std::cout << "PassTester(" << mId << ") copied" << std::endl;
    }

    PassProbe(PassProbe &&passTester) noexcept
    {
        mId = passTester.mId;
        std::cout << "PassTester(" << mId << ") moved" << std::endl;
    }

    ~PassProbe()
    { std::cout << "PassTester(" << mId << ") destructed" << std::endl; }

    int num() const
    { return mId; }

private:
    int mId;
};
}
#endif