#include "EConnection.h"

EConnection::GeneralizedConnection::GeneralizedConnection(
        EObject *signalObject, const std::string& signalId,
        EObject *slotObject, const std::string& slotId,
        ConnectionType connectionType)
        :
        mSignalObject(signalObject), mSignalId(signalId),
        mSlotObject(slotObject), mSlotId(slotId),
        mConnectionType(connectionType)
{
    mCallCount = 0;
}
