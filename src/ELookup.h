#ifndef ETHREAD2_ELOOKUP_H
#define ETHREAD2_ELOOKUP_H

#include <vector>
#include <shared_mutex>
#include <map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <optional>
#include <typeindex>
#include <graphviz/gvc.h>
#include "EConnection.h"

// forward declaration
class EThread;

struct ELookup
{
// singleton
public:
    static ELookup& instance()
    {
        static ELookup instance;
        return instance;
    }
    ELookup(ELookup const&) = delete;
    void operator=(ELookup const&) = delete;
private:
    ELookup() {}
    ~ELookup();

// struct
public:
    std::vector<EObject*> mObjectList; // map of active objects and their threads in affinity
    std::vector<std::unique_ptr<EConnection::GeneralizedConnection>> mConnectionGraph; // edge list of active connection graph
    void unprotectedAddObjectList(EObject* object);
    void unprotectedRemoveObjectThreadMap(EObject* object);
    void unprotectedAddConnection(std::unique_ptr<EConnection::GeneralizedConnection>&& connection);
    void unprotectedRemoveObjectConnection(EObject* object);
    std::shared_mutex& getGlobalMutex();
    void dumpConnectionGraph(const std::string& fileName, bool showHiddenConnections = false);
private:
    std::shared_mutex mGlobalMutex;
};

#endif
