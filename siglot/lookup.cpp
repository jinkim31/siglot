#include "lookup.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include "object.h"

siglot::Lookup::~Lookup()
{
    if(!mConnectionGraph.empty())
    {
        std::cerr<<"Connection graph is not empty on lookup destruction. "
                   "Make sure to remove objects before they get destructed."<<std::endl;
    }
    if(!mObjectList.empty())
    {
        std::cerr<<"Object list is not empty on lookup destruction. "
                   "Make sure to remove objects before they get destructed"
                   "("<<mObjectList.size()<<" object(s) in list)."<<std::endl;
    }
}

std::shared_mutex &siglot::Lookup::getGlobalMutex()
{
    return mGlobalMutex;
}

void siglot::Lookup::unprotectedAddObjectList(Object *object)
{
    if(std::find(mObjectList.begin(), mObjectList.end(),object) == mObjectList.end())
        mObjectList.push_back(object);
}

void siglot::Lookup::unprotectedRemoveObjectThreadMap(Object *object)
{
    mObjectList.erase(
            std::remove_if(mObjectList.begin(), mObjectList.end(),
                           [&](auto& iter) {return iter==object;}),
            mObjectList.end());
}

void siglot::Lookup::unprotectedAddConnection(std::unique_ptr<Connection::GeneralizedConnection> &&connection)
{
    mConnectionGraph.push_back(std::move(connection));
}

void siglot::Lookup::unprotectedRemoveObjectConnection(Object *object)
{
    mConnectionGraph.erase(
            std::remove_if(mConnectionGraph.begin(), mConnectionGraph.end(),
                           [&](auto& iter) {return  iter->mSignalObject == object || iter->mSlotObject == object;}),
            mConnectionGraph.end());
}

void siglot::Lookup::dumpConnectionGraph(const std::string& fileFormat, const std::string &fileName, bool showHiddenConnections)
{
    // create gvc context and graph
    GVC_t *gvc = gvContext();
    Agraph_t *g = agopen("Connection Graph", Agdirected, 0);


    std::shared_lock<std::shared_mutex> lookupLock(Lookup::instance().getGlobalMutex());
    std::unordered_map<Object*, std::string> objectNameMap;
    std::unordered_map<Thread*, std::string> threadNameMap;
    std::stringstream ss;

    // make object, thread names
    for(const auto& object : mObjectList)
    {
        // make object node names
        objectNameMap[object] = object->name();

        // thread subgraph name
        ss << object->mThreadInAffinity->mName
           << "\n(" << std::hex << object->mThreadInAffinity << ")\n"
           << object->mThreadInAffinity->mEventQueue.size()<<" events in queue";
        threadNameMap[object->mThreadInAffinity] = ss.str();
        ss.str(""); ss.clear();
    }


    // make subgraphs
    std::unordered_map<Thread*, Agraph_t*> threadMap;
    for(const auto& threadNamePair : threadNameMap)
    {
        auto subgraph = agsubg(g,(char*)("cluster" + threadNamePair.second).c_str(), 1);
        agsafeset(subgraph, "label", (char*)threadNamePair.second.c_str(), "");
        threadMap[threadNamePair.first] = subgraph;
    }

    // make object nodes
    std::unordered_map<Object*, Agnode_t*> nodeMap;
    for(const auto& objectClassNamePair : objectNameMap)
    {
        ss << objectClassNamePair.second << "\n(" << std::hex << objectClassNamePair.first << ")";
        auto node = agnode(threadMap[objectClassNamePair.first->mThreadInAffinity], &ss.str()[0], 1);
        ss.str(""); ss.clear();
        agsafeset(node, "shape", "box", "");
        agsafeset(node, "style", "filled", "");
        nodeMap[objectClassNamePair.first] = node;
    }

    // make signal slot nodes and edges
    for(const auto& connection : mConnectionGraph)
    {
        if(connection->mIsHiddenInGraphViz && !showHiddenConnections)
            continue;

        // signal node
        ss << connection->mSignalId << "(" << std::hex << connection->mSignalObject << ")";
        auto signalNode = agnode(threadMap[connection->mSignalObject->mThreadInAffinity], (char*)ss.str().c_str(), 1);
        agsafeset(signalNode, "color", "blue", "");
        agsafeset(signalNode, "label", connection->mSignalId.c_str(), "");
        ss.str(""), ss.clear();

        // object-signal edge
        agedge(g, nodeMap[connection->mSignalObject], signalNode, "", 1);

        // slot node
        ss << connection->mSlotId << "(" << std::hex << connection->mSlotObject << ")";
        auto slotNode = agnode(threadMap[connection->mSlotObject->mThreadInAffinity], (char*)ss.str().c_str(), 1);
        agsafeset(slotNode, "color", "orange", "");
        agsafeset(slotNode, "label", connection->mSlotId.c_str(), "");
        ss.str(""), ss.clear();

        // slot-object edge
        agedge(g, slotNode, nodeMap[connection->mSlotObject], "", 1);

        // signal-slot edge
        auto signalSlotEdge = agedge(g, signalNode, slotNode, 0, 1);
        ss << std::dec << connection->mCallCount <<" calls\n"<< connection->mCallFrequency <<" Hz";
        agsafeset(signalSlotEdge, "label", ss.str().c_str(), "");
        ss.str(""); ss.clear();
    }

    // Use the directed graph layout engine
    gvLayout(gvc, g, "dot");

    // Output in .dot format
    gvRender(gvc, g, fileFormat.c_str(), fopen(fileName.c_str(), "w"));
    gvFreeLayout(gvc, g);

    agclose(g);
}
