#include "blueprint/NSCompNode.h"
#include "blueprint/CompNode.h"
#include "blueprint/NodeFactory.h"
#include "blueprint/Pins.h"
#include "blueprint/Connecting.h"
#include "blueprint/Node.h"
#include "blueprint/NodeBuilder.h"

#include <node0/SceneNode.h>

#include <map>

namespace bp
{

size_t NSCompNode::GetBinSize(const std::string& dir) const
{
	return 0;
}

void NSCompNode::StoreToBin(const std::string& dir, bs::ExportStream& es) const
{
}

void NSCompNode::LoadFromBin(const std::string& dir, bs::ImportStream& is)
{
}

void NSCompNode::StoreToJson(const std::string& dir, rapidjson::Value& val, rapidjson::MemoryPoolAllocator<>& alloc) const
{
	val.SetObject();

	val.AddMember("node_type", rapidjson::Value(m_node->GetClassInfo().GetClassName().c_str(), alloc), alloc);

	rapidjson::Value node_val;
	m_node->StoreToJson(dir, node_val, alloc);
	val.AddMember("node_val", node_val, alloc);
}

void NSCompNode::LoadFromJson(mm::LinearAllocator& alloc, const std::string& dir, const rapidjson::Value& val)
{
	std::string type = val["node_type"].GetString();
	m_node = NodeFactory::Instance()->Create(type);
	m_node->LoadFromJson(alloc, dir, val["node_val"]);
}

void NSCompNode::StoreToMem(CompNode& comp) const
{
	comp.SetNode(m_node);
}

void NSCompNode::LoadFromMem(const CompNode& comp)
{
	m_node = comp.GetNode();
}

void NSCompNode::StoreConnection(const std::vector<n0::SceneNodePtr>& nodes,
	                             rapidjson::Value& nodes_val,
	                             rapidjson::MemoryPoolAllocator<>& alloc)
{
	assert(nodes_val.Size() == nodes.size());

	std::map<const Node*, int> node2idx;
	int idx = 0;
	for (auto& node : nodes)
	{
		auto& bp_node = node->GetUniqueComp<CompNode>();
		node2idx.insert({ bp_node.GetNode().get(), idx++ });
	}

	for (int i = 0, n = nodes.size(); i < n; ++i)
	{
		auto& bp_node = nodes[i]->GetUniqueComp<CompNode>();

		int comp_idx = -1;
		for (int j = 0, m = nodes_val[i].Size(); j < m; ++j) {
			if (nodes_val[i][j]["comp_type"] == "bp_node") {
				comp_idx = j;
				break;
			}
		}
		assert(comp_idx != -1);
		auto& bp_node_val = nodes_val[i][comp_idx];

		rapidjson::Value outputs_val;
		outputs_val.SetArray();
		auto& outputs = bp_node.GetNode()->GetAllOutput();
		for (auto& output : outputs)
		{
			rapidjson::Value output_val;
			output_val.SetObject();
			output_val.AddMember("from", rapidjson::Value(output->GetName().c_str(), alloc), alloc);

			rapidjson::Value conns_val;
			conns_val.SetArray();
			auto& conns = output->GetConnecting();
			for (auto& conn : conns)
			{
				auto& to_pin = conn->GetTo();
				assert(to_pin);

				rapidjson::Value conn_val;
				conn_val.SetObject();

				auto itr = node2idx.find(&to_pin->GetParent());
				assert(itr != node2idx.end());
				conn_val.AddMember("node_idx", itr->second, alloc);

				conn_val.AddMember("pin", rapidjson::Value(to_pin->GetName().c_str(), alloc), alloc);

				conns_val.PushBack(conn_val, alloc);
			}

			output_val.AddMember("to", conns_val, alloc);

			outputs_val.PushBack(output_val, alloc);
		}
		bp_node_val.AddMember("connection", outputs_val, alloc);
	}
}

void NSCompNode::LoadConnection(std::vector<n0::SceneNodePtr>& nodes,
	                            const rapidjson::Value& nodes_val)
{
	assert(nodes_val.Size() == nodes.size());

	std::vector<Node*> bp_nodes;
	bp_nodes.reserve(nodes.size());
	for (auto& node : nodes) {
		auto& bp_node = node->GetUniqueComp<CompNode>();
		bp_nodes.push_back(bp_node.GetNode().get());
	}

	auto builder = bp::NodeBuilder::Instance();

	int src_node_idx = 0;
	for (auto& node_val : nodes_val.GetArray())
	{
		int comp_idx = -1;
		for (int i = 0, n = node_val.Size(); i < n; ++i) {
			if (node_val[i]["comp_type"] == "bp_node") {
				comp_idx = i;
				break;
			}
		}
		assert(comp_idx != -1);
		auto& outputs_val = node_val[comp_idx]["connection"];
		for (auto& output_val : outputs_val.GetArray())
		{
			auto src_name = output_val["from"].GetString();
			auto src = QueryPinsByName(*nodes[src_node_idx]->GetUniqueComp<CompNode>().GetNode(), false, src_name);

			auto& conns_val = output_val["to"];
			for (auto& conn_val : conns_val.GetArray())
			{
				int node_idx = conn_val["node_idx"].GetInt();
				auto dst_name = conn_val["pin"].GetString();
				auto dst = QueryPinsByName(*nodes[node_idx]->GetUniqueComp<CompNode>().GetNode(), true, dst_name);

				builder->BeforeConnected(*src, *dst);
				make_connecting(src, dst);
				builder->AfterConnected(*src, *dst);
			}
		}

		++src_node_idx;
	}
}

std::shared_ptr<Pins> NSCompNode::QueryPinsByName(const Node& node, bool is_input, const std::string& name)
{
	auto& array = is_input ? node.GetAllInput() : node.GetAllOutput();
	for (auto& pins : array) {
		if (pins->GetName() == name) {
			return pins;
		}
	}
	return nullptr;
}

}