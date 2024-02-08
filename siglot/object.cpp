#include "object.h"

siglot::Object::Object()
{
    mName = "Object";
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
    Lookup::instance().unprotectedRemoveObjectThreadMap(this);
    Lookup::instance().unprotectedRemoveObjectConnection(this);
    lock.unlock();
    onRemove();
}

void siglot::Object::setName(const std::string &name)
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    mName = name;
}

std::string siglot::Object::name()
{
    std::shared_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    return mName;
}
