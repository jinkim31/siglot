#include "EObject.h"

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