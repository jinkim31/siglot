#include "EConnection.h"

EConnection::GeneralizedConnection::GeneralizedConnection(
        EObject *signalObject, const std::string& signalId,
        EObject *slotObject, const std::string& slotId,
        ConnectionType connectionType, bool isHiddenInGraphViz)
        :
        mSignalObject(signalObject), mSignalId(signalId),
        mSlotObject(slotObject), mSlotId(slotId),
        mConnectionType(connectionType), mIsHiddenInGraphViz(isHiddenInGraphViz)
{
    mCallCount = 0;
}
