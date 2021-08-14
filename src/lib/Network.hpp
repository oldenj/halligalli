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

#ifndef __NETWORK_HPP
#define __NETWORK_HPP

#include <functional>
#include <map>
//#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <boost/container_hash/hash.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "loguru.hpp"

#include "Arc.hpp"
#include "Node.hpp"
#include "Path.hpp"

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
		boost::property<boost::vertex_distance_t, double>, boost::property<boost::edge_weight_t, double> > dag_t;
typedef boost::property_map<dag_t, boost::vertex_distance_t>::type distance_map_t;

/**
 * @brief Represents a boost edge and supports ordering such that it may be used as a key
 * @author Jurek Olden (jurek.olden@in.tum.de)
 *
 * We differentiate between 'arcs' as the mathematical object and 'edges' as the concrete implementation using boost::edge.
 * This Edge class is only used within the Network class, so there should not be any confusion.
 */
struct Edge {
	dag_t::vertex_descriptor source;
	dag_t::vertex_descriptor target;

	bool operator<(const Edge& rhs) const;
	bool operator==(const Edge& rhs) const;
};

namespace std {
	template<> struct hash<Edge> {
		std::size_t operator()(const Edge& edge) const noexcept {
			std::size_t seed = 0;
			boost::hash_combine(seed, edge.source);
			boost::hash_combine(seed, edge.target);
			return seed;
		}
	};
}

/** Represents a network and supports associated operations such as calculating the shortest path and manipulating arc weights */
class Network {
	public:
		Network(const std::string &filename, std::vector<Arc>& arc_list);
		Path shortest_path();

		void reset_edge_weights();
		void set_edge_weight(const Arc& arc, double weight);
		void add_to_edge_weight(const Arc& arc, double weight);

		// these can be const and atomic because the graph does not change after initialization
		int get_group() const { return group_; }
		int get_vertex_count() const { return boost::num_vertices(graph_); }
		int get_edge_count() const { return boost::num_edges(graph_); }
		// this shouldn't be const because we need to mutex the graph (to stay atomic, in case of async weight manipulation)
		int get_edge_weight(const Arc& arc);

	private:

		int group_;
		Path shortest_path_;
		bool shortest_path_is_fresh_;
		//std::mutex graph_mutex_;
		dag_t graph_;
		dag_t::vertex_descriptor source_;
		dag_t::vertex_descriptor sink_;
		std::map<int, std::pair<dag_t::vertex_descriptor, Node>> id_map_;
		std::unordered_map<Arc, Edge> arc_to_edge_map_;
		std::unordered_map<Edge, Arc> edge_to_arc_map_;
};

#endif
