#include "EObject.h"

EObject::EObject()
{
    mName = "Object";
}

void EObject::move(EThread& ethread)
{
    std::unique_lock<std::shared_mutex> lock(ELookup::instance().getGlobalMutex());
    ELookup::instance().unprotectedAddObjectList(this);
    mThreadInAffinity = &ethread;
    lock.unlock();
    onMove(ethread);
}

void EObject::remove()
{
    std::unique_lock<std::shared_mutex> lock(ELookup::instance().getGlobalMutex());
    ELookup::instance().unprotectedRemoveObjectThreadMap(this);
    ELookup::instance().unprotectedRemoveObjectConnection(this);
    lock.unlock();
    onRemove();
}

void EObject::setName(const std::string &name)
{
    std::unique_lock<std::shared_mutex> lock(ELookup::instance().getGlobalMutex());
    mName = name;
}

std::string EObject::name()
{
    std::shared_lock<std::shared_mutex> lock(ELookup::instance().getGlobalMutex());
    return mName;
}
