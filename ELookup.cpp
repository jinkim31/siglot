#include "ELookup.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include "EObject.h"

ELookup::~ELookup()
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

std::shared_mutex &ELookup::getGlobalMutex()
{
    return mGlobalMutex;
}

void ELookup::unprotectedAddObjectList(EObject *object)
{
    if(std::find(mObjectList.begin(), mObjectList.end(),object) == mObjectList.end())
        mObjectList.push_back(object);
}

void ELookup::unprotectedRemoveObjectThreadMap(EObject *object)
{
    mObjectList.erase(
            std::remove_if(mObjectList.begin(), mObjectList.end(),
                           [&](auto& iter) {return iter==object;}),
            mObjectList.end());
}

void ELookup::unprotectedAddConnection(std::unique_ptr<EConnection::GeneralizedConnection> &&connection)
{
    mConnectionGraph.push_back(std::move(connection));
}

void ELookup::unprotectedRemoveObjectConnection(EObject *object)
{
    mConnectionGraph.erase(
            std::remove_if(mConnectionGraph.begin(), mConnectionGraph.end(),
                           [&](auto& iter) {return  iter->mSignalObject == object || iter->mSlotObject == object;}),
            mConnectionGraph.end());
}

void ELookup::dumpConnectionGraph(const std::string &fileName, bool showHiddenConnections)
{
    std::shared_lock<std::shared_mutex> lookupLock(ELookup::instance().getGlobalMutex());
    std::unordered_map<EObject*, std::string> objectClassNameMap;
    for(const auto& connection : mConnectionGraph)
    {
        if(connection->mIsHiddenInGraphViz && !showHiddenConnections)
            continue;
        objectClassNameMap[connection->mSignalObject] = connection->mSignalObject->name();
        objectClassNameMap[connection->mSlotObject] = connection->mSlotObject->name();
    }

    // create gvc context and graph
    GVC_t *gvc = gvContext();
    Agraph_t *g = agopen("Connection Graph", Agdirected, 0);

    // make object nodes
    std::unordered_map<EObject*, Agnode_t*> nodeMap;
    for(const auto& objectClassNamePair : objectClassNameMap)
    {
        std::stringstream ss;
        ss << objectClassNamePair.second << "(" << std::hex << objectClassNamePair.first << ")";
        auto node = agnode(g, &ss.str()[0], 1);
        agsafeset(node, "shape", "box", "");
        nodeMap[objectClassNamePair.first] = node;
    }

    // make signal slot nodes and edges
    std::vector<Agnode_t*> signalNodes, slotNodes;
    std::stringstream ss;
    for(const auto& connection : mConnectionGraph)
    {
        if(connection->mIsHiddenInGraphViz && !showHiddenConnections)
            continue;

        // signal node
        ss << connection->mSignalId << "(" << std::hex << connection->mSignalObject << ")";
        auto signalNode = agnode(g, (char*)ss.str().c_str(), 1);
        agsafeset(signalNode, "color", "blue", "");
        ss.str(""), ss.clear();
        // object-signal edge
        agedge(g, nodeMap[connection->mSignalObject], signalNode, "", 1);
        // slot node
        ss << connection->mSlotId << "(" << std::hex << connection->mSlotObject << ")";
        auto slotNode = agnode(g, (char*)ss.str().c_str(), 1);
        agsafeset(slotNode, "color", "orange", "");
        ss.str(""), ss.clear();
        // slot-object edge
        agedge(g, slotNode, nodeMap[connection->mSlotObject], "", 1);
        // signal-slot edge
        agedge(g, signalNode, slotNode, 0, 1);
    }

    // Use the directed graph layout engine
    gvLayout(gvc, g, "dot");

    // Output in .dot format
    gvRender(gvc, g, "png", fopen("test.png", "w"));
    gvFreeLayout(gvc, g);

    agclose(g);
}
