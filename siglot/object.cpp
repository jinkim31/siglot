#include "object.h"

siglot::Object::Object() : mID(generateID())
{
    mSiglotObjectName = "Object";
    mThreadInAffinity = nullptr;
}

void siglot::Object::move(Thread& ethread)
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    Lookup::instance().unprotectedAddObjectList(this);
    mThreadInAffinity = &ethread;
    lock.unlock();
    onMove(ethread);
}

void siglot::Object::remove()
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    Lookup::instance().unprotectedRemoveObjectList(this);
    Lookup::instance().unprotectedRemoveObjectConnection(this);
    lock.unlock();

    onRemove();
}

void siglot::Object::setSiglotObjectName(const std::string &name)
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    mSiglotObjectName = name;
}

std::string siglot::Object::siglotObjectName()
{
    std::shared_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    return mSiglotObjectName;
}

void siglot::Object::handleNextEventsFirst()
{
    mThreadInAffinity->handleEvents();
}
