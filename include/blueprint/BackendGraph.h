#pragma once

#include "blueprint/typedef.h"

#include <dag/Node.h>
#include <dag/Graph.h>
#include <node0/typedef.h>

#include <boost/noncopyable.hpp>

namespace bp
{

class Connecting;

template <typename T>
class BackendGraph : boost::noncopyable
{
public:
    BackendGraph(const std::string& back_name, const std::string& front_name);

    // update nodes
    void OnAddNode(const Node& node, const n0::SceneNodePtr& snode,
        bool need_update = true);
    void OnRemoveNode(const Node& node);
    void OnClearAllNodes();

    // update node prop
    void OnNodePropChanged(const NodePtr& node);

    // update node conn
    void OnConnected(const Connecting& conn);
    void OnDisconnecting(const Connecting& conn);
    void OnRebuildConnection();

    auto& GetEval() const { return m_eval; }

    std::shared_ptr<dag::Node<T>>
        QueryBackNode(const Node& front_node) const;

private:
    void Update();

    void UpdatePropBackFromFront(const bp::Node& front, dag::Node<T>& back);

    std::shared_ptr<dag::Node<T>>
        CreateBackFromFront(const bp::Node& front);

private:
    std::string m_back_name;
    std::string m_front_name;

    dag::Graph<T> m_eval;

    std::unordered_map<const Node*, std::shared_ptr<dag::Node<T>>> m_front2back;

}; // BackendGraph

}

#include "blueprint/BackendGraph.inl"