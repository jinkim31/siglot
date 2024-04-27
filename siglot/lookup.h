#ifndef SIGLOT_LOOKUP_H
#define SIGLOT_LOOKUP_H

#include <vector>
#include <shared_mutex>
#include <map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <optional>
#include <typeindex>
#include "connection.h"
#ifdef SIGLOT_WITH_GRAPHVIZ
#include <graphviz/gvc.h>
#endif


namespace siglot
{
// forward declaration
class Thread;

struct Lookup
{
// singleton
public:
    static Lookup &instance()
    {
        static Lookup instance;
        return instance;
    }

    Lookup(Lookup const &) = delete;
    void operator=(Lookup const &) = delete;
private:
    Lookup()
    {}

    ~Lookup();

// struct
public:
    void
    dumpConnectionGraph(const std::string &fileFormat, const std::string &fileName, bool showHiddenConnections = false);
private:
    std::unordered_map<size_t, Object*> mObjectList; // map of object id and object pointer
    std::vector<std::unique_ptr<Connection::GeneralizedConnection>> mConnectionGraph; // edge list of active connection graph
    void unprotectedAddObjectList(Object *object);
    void unprotectedRemoveObjectList(Object *object);
    void unprotectedAddConnection(std::unique_ptr<Connection::GeneralizedConnection> &&connection);
    void unprotectedRemoveObjectConnection(Object *object);
    std::shared_mutex &getGlobalMutex();
    std::shared_mutex mGlobalMutex;

    friend class Object;

    friend class Thread;
};
}
#endif
