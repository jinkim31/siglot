#include "connection.h"
#include "util.h"

siglot::Connection::GeneralizedConnection::GeneralizedConnection(size_t signalObjectID, siglot::Object *signalObject,
                                                                 const std::string &signalName, size_t slotObjectID,
                                                                 siglot::Object *slotObject,
                                                                 const std::string &slotName,
                                                                 siglot::Connection::ConnectionType connectionType,
                                                                 bool isHiddenInGraphViz)
        :
        mSignalObjectID(signalObjectID), mSignalObject(signalObject), mSignalName(signalName), mSignalId(functionNameWithNamespaceToSiglotId(signalName)),
        mSlotObjectID(slotObjectID), mSlotObject(slotObject), mSlotName(slotName), mSlotId(functionNameWithNamespaceToSiglotId(slotName)),
        mConnectionType(connectionType), mIsHiddenInGraphViz(isHiddenInGraphViz)
{
    mCallCount = 0;
    mCallFrequency = 0.0f;
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    mLastCallTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}