#include "object.h"

Object::Object()
{
    mName = "Object";
}

void Object::move(Thread& ethread)
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    Lookup::instance().unprotectedAddObjectList(this);
    mThreadInAffinity = &ethread;
    lock.unlock();
    onMove(ethread);
}

void Object::remove()
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    Lookup::instance().unprotectedRemoveObjectThreadMap(this);
    Lookup::instance().unprotectedRemoveObjectConnection(this);
    lock.unlock();
    onRemove();
}

void Object::setName(const std::string &name)
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    mName = name;
}

std::string Object::name()
{
    std::shared_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    return mName;
}
