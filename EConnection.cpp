#include "EConnection.h"

EConnection::GeneralizedConnection::GeneralizedConnection(
        EObject *signalObject, std::type_index signalId,
        EObject *slotObject, std::type_index slotId,
        ConnectionType connectionType)
        :
        mSignalObject(signalObject), mSignalId(signalId),
        mSlotObject(slotObject), mSlotId(slotId),
        mConnectionType(connectionType)
{
}
