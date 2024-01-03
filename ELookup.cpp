#include "ELookup.h"
#include <algorithm>
#include <iostream>

std::shared_mutex &ELookup::getMutex()
{
    return mMutex;
}

void ELookup::addObjectThreadMap(EObject *object, EThread *thread)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mObjectThreadMap[object] = thread;
    std::cout<<"object thread map added "<<mObjectThreadMap.size()<<std::endl;
}

void ELookup::removeObjectThreadMap(EObject *object)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    auto iter = mObjectThreadMap.find(object) ;
    if( iter != mObjectThreadMap.end() )
        std::cerr<<"removeObjectThreadMap() object not found"<<std::endl;
    mObjectThreadMap.erase( iter );
}

void ELookup::addConnection(std::unique_ptr<GeneralizedConnection> &&connection)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mConnectionGraph.push_back(std::move(connection));
}

void ELookup::removeObjectConnection(EObject *object)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mConnectionGraph.erase(
            std::remove_if(mConnectionGraph.begin(), mConnectionGraph.end(),
                           [&](auto& iter) {return  iter->mSignalObject == object;}),
            mConnectionGraph.end());
}
