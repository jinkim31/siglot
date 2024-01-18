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
    std::shared_mutex mMutex;
    std::unordered_map<EObject*, EThread*> mObjectThreadMap; // map of active objects and their threads in affinity
    std::vector<std::unique_ptr<EConnection::GeneralizedConnection>> mConnectionGraph; // edge list of active connection graph
    std::optional<EThread*> searchObjectThreadMap(EObject* object);
    void addObjectThreadMap(EObject* object, EThread* thread);
    void removeObjectThreadMap(EObject* object);
    void addConnection(std::unique_ptr<EConnection::GeneralizedConnection>&& connection);
    void removeObjectConnection(EObject* object);
    std::shared_mutex& getMutex();
};

#endif
