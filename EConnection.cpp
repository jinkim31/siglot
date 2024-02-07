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
    mCallFrequency = 0.0f;
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    mLastCallTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}
