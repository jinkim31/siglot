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
    if(!mObjectThreadMap.empty())
    {
        std::cerr<<"Object-thread map is not empty on lookup destruction. "
                   "Make sure to remove objects before they get destructed."<<std::endl;
    }
}

std::shared_mutex &ELookup::getMutex()
{
    return mMutex;
}

void ELookup::addObjectThreadMap(EObject *object, EThread *thread)
{
    mObjectThreadMap[object] = thread;
    //std::cout<<"object thread map added "<<mObjectThreadMap.size()<<std::endl;
}

void ELookup::removeObjectThreadMap(EObject *object)
{
    auto iter = mObjectThreadMap.find(object) ;
    if( iter == mObjectThreadMap.end() )
        std::cerr<<"removeObjectThreadMap() object not found"<<std::endl;
    mObjectThreadMap.erase( iter );
}

void ELookup::addConnection(std::unique_ptr<EConnection::GeneralizedConnection> &&connection)
{
    mConnectionGraph.push_back(std::move(connection));
}

void ELookup::removeObjectConnection(EObject *object)
{
    mConnectionGraph.erase(
            std::remove_if(mConnectionGraph.begin(), mConnectionGraph.end(),
                           [&](auto& iter) {return  iter->mSignalObject == object || iter->mSlotObject == object;}),
            mConnectionGraph.end());
}

std::optional<EThread*> ELookup::searchObjectThreadMap(EObject* object)
{
    auto iter = mObjectThreadMap.find(object);
    if(iter == mObjectThreadMap.end())
        return std::nullopt;
    return iter->second;
}
