/**
 * @file
 * @author Jurek Olden (jurek.olden@in.tum.de)
 *
 * @section LICENSE
 *
 * Copyright (C) 2021 Jurek Olden
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Network.hpp"

#include <tuple>

#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>

using boost::property_tree::ptree;

Network::Network(const std::string &filename, std::vector<Arc>& arc_list) {
	shortest_path_is_fresh_ = false;
	// Source and sink flags in order to recognize if no source/sink was found
	bool has_source = false;
	bool has_sink = false;

	// Figure out the graph 'group' from the filename
	std::string group_s = filename.substr(filename.find_last_of("_") + 1);
	try {
		group_ = std::stoi(group_s);
	} catch (const std::exception& e) {
		ABORT_F("Encountered a malformed graph filename (%s): %s", filename.c_str(), e.what());
	}

	// Create empty property tree object
	ptree pt;

	// Parse the XML into the property tree.
	read_xml(filename, pt);

	// TODO templating should be implemented here (with config file)
	// descend to graphml.graph where nodes and edges are stored and iterate over them
	for (const auto &elem : pt.get_child("graphml.graph")) {
		if(elem.first == "node") {
			auto v = add_vertex(graph_);

			int k0, k1, k2, k3, k4, k6;
			bool k5;

			for (const auto &data : elem.second) {
				// skip <xmlattr> content
				if (data.first != "data")
					continue;
				auto key = data.second.get_optional<std::string>("<xmlattr>.key");
				// key not found (original lp files)
				if (!key)
					continue;
				if (key->compare("key0") == 0) {
					k0 = data.second.get_value<int>();
				} else if (key->compare("key1") == 0) {
					k1 = data.second.get_value<int>();
				} else if (key->compare("key2") == 0) {
					k2 = data.second.get_value<int>();
				} else if (key->compare("key3") == 0) {
					k3 = data.second.get_value<int>();
				} else if (key->compare("key4") == 0) {
					k4 = data.second.get_value<int>();
				} else if (key->compare("key5") == 0) {
					if (data.second.get_value<std::string>().compare("true") == 0) {
						k5 = true;
					} else {
						k5 = false;
					}
				} else if (key->compare("key6") == 0) {
					k6 = data.second.get_value<int>();
				} else {
					ABORT_F("Found malformed attribute in Graph XML file (%s): %s", filename.c_str(), key.get().c_str());
				}
			}

			Node node{k0, k1, k2, k3, k4, k5, k6};

			// save the external node id to vertex descriptor mapping for creating edges later on
			id_map_.insert(std::make_pair(elem.second.get<int>("<xmlattr>.id"), std::make_pair(v, node)));

			// check if this is a source or target vertex
			if (k0 == 0 && k1 == 0 && k3 == -1 && k4 == 0 && !k5) {
				if (k2 == -2) {
					if (has_source) ABORT_F("Graph contains more than one source vertex: %s", filename.c_str());
					has_source = true;
					source_ = v;
				} else if (k2 == 35) {
					if (has_sink) ABORT_F("Graph contains more than one sink vertex: %s", filename.c_str());
					has_sink = true;
					sink_ = v;
				}
			}
		}
		if (elem.first == "edge") {
			auto source_id = elem.second.get<int>("<xmlattr>.source");
			auto source_res = id_map_.find(source_id);
			if (!source_res->first) ABORT_F("Graph XML (%s) contains an edge without corresponding source or edge was defined before vertices, source vertex id: %i",
					filename.c_str(), source_id);
			auto source_vertex = source_res->second.first;
			auto source_node = source_res->second.second;

			auto target_id = elem.second.get<int>("<xmlattr>.target");
			auto target_res = id_map_.find(target_id);
			if (!target_res->first) ABORT_F("Graph XML (%s) contains an edge without corresponding target or edge was defined before vertices, target vertex id: %i",
					filename.c_str(), target_id);
			auto target_vertex = target_res->second.first;
			auto target_node = target_res->second.second;

			add_edge(source_vertex, target_vertex, 0, graph_);

			Arc arc{source_node, target_node};
			Edge e{source_vertex, target_vertex};

			arc_to_edge_map_.insert(std::make_pair(arc, e));
			edge_to_arc_map_.insert(std::make_pair(e, arc));
			arc_list.push_back(arc);
		}
	}
	if (!has_source || !has_sink) ABORT_F("Graph is missing source or sink vertex: %s", filename.c_str());
}

Path Network::shortest_path() {
	//const std::lock_guard<std::mutex> lock(graph_mutex_);
	if (shortest_path_is_fresh_) return shortest_path_;

	// https://www.boost.org/doc/libs/1_72_0/libs/graph/example/dag_shortest_paths.cpp
	distance_map_t d_map = get(boost::vertex_distance, graph_);
	boost::property_map<dag_t, boost::edge_weight_t>::type w_map = boost::get(boost::edge_weight, graph_);
	std::vector<boost::default_color_type> color(num_vertices(graph_));
	std::vector<std::size_t> pred(boost::num_vertices(graph_));
  	boost::default_dijkstra_visitor vis;
	std::less<double> compare;
	boost::closed_plus<double> combine;
	// O(V + E)
	boost::dag_shortest_paths(graph_, source_, d_map, w_map, color.data(), pred.data(), vis, compare, combine, (std::numeric_limits<int>::max)(), 0);

	// build path
	// TODO config option to skip this if length non negative
	std::vector<Arc> shortest_path_arcs;
	auto v = sink_;
	while (v != source_) {
		auto result = boost::edge(pred[v], v, graph_);
		assert(result.second);
		auto edge_descriptor = result.first;
		Edge edge = {boost::source(edge_descriptor, graph_), boost::target(edge_descriptor, graph_)};
		auto arc_res = edge_to_arc_map_.find(edge);
		if (arc_res == edge_to_arc_map_.end()) ABORT_F("Shortest path contained an edge without corresponding arc, network group: %i", group_);
		shortest_path_arcs.push_back(arc_res->second);
		v = pred[v];
	}
	Path p{d_map[sink_], shortest_path_arcs, group_};
	shortest_path_ = p;
	shortest_path_is_fresh_ = true;
	return p;
}

int Network::get_edge_weight(const Arc& arc) {
	// if we want to stay fully atomic this needs to mutex
	//const std::lock_guard<std::mutex> lock(graph_mutex_);
	auto edge_res = arc_to_edge_map_.find(arc);
	if (edge_res == arc_to_edge_map_.end())
		ABORT_F("Tried to get edge weight of an arc (%s) without corresponding edge, or the arc to network mapping was wrong", arc.to_string().c_str());
	auto edge = edge_res->second;
	auto boost_edge = boost::edge(edge.source, edge.target, graph_);
	if (!boost_edge.second) ABORT_F("Could not find a boost edge from boost source and target");
	return get(boost::edge_weight, graph_, boost_edge.first);
}

void Network::reset_edge_weights() {
	//const std::lock_guard<std::mutex> lock(graph_mutex_);
	shortest_path_is_fresh_ = false;
	boost::property_map<dag_t, boost::edge_weight_t>::type edge_weight_map = get(boost::edge_weight, graph_);
	boost::graph_traits<dag_t>::edge_iterator e_it, e_end;
	for (std::tie(e_it, e_end) = boost::edges(graph_); e_it != e_end; ++e_it) {
		boost::put(boost::edge_weight_t(), graph_, *e_it, 0);
	}
}

void Network::set_edge_weight(const Arc& arc, double weight) {
	//const std::lock_guard<std::mutex> lock(graph_mutex_);
	shortest_path_is_fresh_ = false;
	auto edge_res = arc_to_edge_map_.find(arc);
	if (edge_res == arc_to_edge_map_.end())
		ABORT_F("Tried to set edge weight on an arc (%s) without corresponding edge, or the arc to network mapping was wrong", arc.to_string().c_str());
	auto edge = edge_res->second;
	auto boost_edge = boost::edge(edge.source, edge.target, graph_);
	if (!boost_edge.second) ABORT_F("Could not find a boost edge from boost source and target");
	boost::put(boost::edge_weight_t(), graph_, boost_edge.first, weight);
}

void Network::add_to_edge_weight(const Arc& arc, double weight) {
	//const std::lock_guard<std::mutex> lock(graph_mutex_);
	shortest_path_is_fresh_ = false;
	auto edge_res = arc_to_edge_map_.find(arc);
	if (edge_res == arc_to_edge_map_.end())
		ABORT_F("Tried to add to edge weight on an arc (%s) without corresponding edge, or the arc to network mapping was wrong", arc.to_string().c_str());
	auto edge = edge_res->second;
	auto boost_edge = boost::edge(edge.source, edge.target, graph_);
	if (!boost_edge.second) ABORT_F("Could not find a boost edge from boost source and target");
	auto current_weight = get(boost::edge_weight, graph_, boost_edge.first);
	boost::put(boost::edge_weight_t(), graph_, boost_edge.first, current_weight + weight);
}

bool Edge::operator<(const Edge& rhs) const {
	return std::tie(source, target) <
		std::tie(rhs.source, rhs.target);
}

bool Edge::operator==(const Edge& rhs) const {
	return
		source == rhs.source &&
		target == rhs.target;
}
