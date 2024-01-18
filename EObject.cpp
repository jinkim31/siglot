#include "EObject.h"

void EObject::move(EThread& ethread)
{
    std::unique_lock<std::shared_mutex> lock(ELookup::instance().getMutex());
    ELookup::instance().addObjectThreadMap(this, &ethread);
    lock.unlock();
    onMove(ethread);
}

void EObject::remove()
{
    std::unique_lock<std::shared_mutex> lock(ELookup::instance().getMutex());
    ELookup::instance().removeObjectThreadMap(this);
    ELookup::instance().removeObjectConnection(this);
    lock.unlock();
    onRemove();
}