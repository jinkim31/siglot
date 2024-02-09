#include "connection.h"
#include "util.h"

siglot::Connection::GeneralizedConnection::GeneralizedConnection(
        Object *signalObject, const std::string& signalName,
        Object *slotObject, const std::string& slotName,
        ConnectionType connectionType, bool isHiddenInGraphViz)
        :
        mSignalObject(signalObject), mSignalName(signalName), mSignalId(functionNameWithNamespaceToSiglotId(signalName)),
        mSlotObject(slotObject), mSlotName(slotName), mSlotId(functionNameWithNamespaceToSiglotId(slotName)),
        mConnectionType(connectionType), mIsHiddenInGraphViz(isHiddenInGraphViz)
{
    mCallCount = 0;
    mCallFrequency = 0.0f;
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    mLastCallTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}
