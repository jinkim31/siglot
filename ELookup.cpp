#include "ELookup.h"
#include <algorithm>
#include <iostream>

ELookup::~ELookup()
{
    if(!mConnectionGraph.empty())
    {
        std::cerr<<"Connection graph is not empty on lookup destruction. "
                   "Make sure to remove objects before they get destructed."<<std::endl;
    }
    if(!mObjectList.empty())
    {
        std::cerr<<"Object-thread map is not empty on lookup destruction. "
                   "Make sure to remove objects before they get destructed."<<std::endl;
    }
}

std::shared_mutex &ELookup::getGlobalMutex()
{
    return mGlobalMutex;
}

void ELookup::unprotectedAddObjectList(EObject *object)
{
    if(std::find(mObjectList.begin(), mObjectList.end(),object) == mObjectList.end())
        mObjectList.push_back(object);
}

void ELookup::unprotectedRemoveObjectThreadMap(EObject *object)
{
    mObjectList.erase(
            std::remove_if(mObjectList.begin(), mObjectList.end(),
                           [&](auto& iter) {return iter==object;}),
            mObjectList.end());
}

void ELookup::unprotectedAddConnection(std::unique_ptr<EConnection::GeneralizedConnection> &&connection)
{
    mConnectionGraph.push_back(std::move(connection));
}

void ELookup::unprotectedRemoveObjectConnection(EObject *object)
{
    mConnectionGraph.erase(
            std::remove_if(mConnectionGraph.begin(), mConnectionGraph.end(),
                           [&](auto& iter) {return  iter->mSignalObject == object || iter->mSlotObject == object;}),
            mConnectionGraph.end());
}