#ifndef ETHREAD2_ELOOKUP_H
#define ETHREAD2_ELOOKUP_H

#include <vector>
#include <shared_mutex>
#include <map>
#include <memory>
#include <mutex>
#include <algorithm>

// forward declaration
class EThread;
class EObject;

struct GeneralizedConnection{
    virtual ~GeneralizedConnection() = default;
    EObject* mSignalObject, *mSlotObject;
    size_t mSignalHash, mSlotHash;
};

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
    std::map<EObject*, EThread*> mObjectThreadMap; // map of active objects and their threads in affinity
    std::vector<std::unique_ptr<GeneralizedConnection>> mConnectionGraph; // edge list of active connection graph
    void addObjectThreadMap(EObject* object, EThread* thread);
    void removeObjectThreadMap(EObject* object);
    void addConnection(std::unique_ptr<GeneralizedConnection>&& connection);
    void removeObjectConnection(EObject* object);
    std::shared_mutex& getMutex();
};

#endif
