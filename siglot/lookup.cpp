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
                   "Make sure to remove objects before they get destructed."
                   "\nConnections:"<<std::endl;
        for(const auto& connection : mConnectionGraph)
        {
            std::cerr
            <<"  "
            <<connection->mSignalObject->siglotObjectName()<<"("<<connection->mSignalObject->mID<<")"
            <<" > "
            <<connection->mSignalName
            <<" > "
            <<connection->mSlotName
            <<" > "
            <<connection->mSlotObject->siglotObjectName()<<"("<<connection->mSlotObject->mID<<")"
            <<std::endl;
        }
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
    if(mObjectList.find(object->mID) == mObjectList.end())
        mObjectList.insert({object->mID, object});
}

void siglot::Lookup::unprotectedRemoveObjectList(Object *object)
{
    std::erase_if(mObjectList, [&](auto& iter) {return iter.first==object->mID;});
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
#ifdef SIGLOT_WITH_GRAPHVIZ
    // create gvc context and graph
    GVC_t *gvc = gvContext();
    Agraph_t *g = agopen("Connection Graph", Agdirected, 0);
    agsafeset(g, "dpi", "300", "");


    std::shared_lock<std::shared_mutex> lookupLock(Lookup::instance().getGlobalMutex());
    std::unordered_map<Object*, std::string> objectNameMap;
    std::unordered_map<Thread*, std::string> threadNameMap;
    std::stringstream ss;

    // make object, thread names
    for(const auto& idObjectPair : mObjectList)
    {
        auto& object = idObjectPair.second;
        // make object node names
        objectNameMap[object] = object->name();

        // thread subgraph name
        ss << object->mThreadInAffinity->mName
           << "\n(" << object->mThreadInAffinity->mThread.get_id() << ")\n"
           << object->mThreadInAffinity->mEventQueue.size()<<" events in queue";
        threadNameMap[object->mThreadInAffinity] = ss.str();
        ss.str(""); ss.clear();
    }


    // make thread subgraphs
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
        ss << connection->mSignalName << "(" << std::hex << connection->mSignalObject << ")";
        auto signalNode = agnode(threadMap[connection->mSignalObject->mThreadInAffinity], (char*)ss.str().c_str(), 1);
        agsafeset(signalNode, "color", "blue", "");
        agsafeset(signalNode, "label", connection->mSignalName.c_str(), "");
        ss.str(""), ss.clear();

        // object-signal edge
        agedge(g, nodeMap[connection->mSignalObject], signalNode, "", 1);

        // slot node
        ss << connection->mSlotName << "(" << std::hex << connection->mSlotObject << ")";
        auto slotNode = agnode(threadMap[connection->mSlotObject->mThreadInAffinity], (char*)ss.str().c_str(), 1);
        agsafeset(slotNode, "color", "orange", "");
        agsafeset(slotNode, "label", connection->mSlotName.c_str(), "");
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
#endif
}
