#include "EObject.h"

void EObject::move(EThread& ethread)
{
    ELookup::instance().addObjectThreadMap(this, &ethread);
}

void EObject::remove()
{
    ELookup::instance().removeObjectThreadMap(this);
    ELookup::instance().removeObjectConnection(this);
}