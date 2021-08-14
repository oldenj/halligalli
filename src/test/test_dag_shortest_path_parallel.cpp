#include <future>
#include <iostream>
#include <string>

#include <boost/graph/dag_shortest_paths.hpp>

#include "catch.hpp"
#include "types.hpp"
#include "read_xml.hpp"

/*
 * The idea here is to just duplicate the same graph as many times as it has nodes,
 * then compute a distance map for each possible source node in parallel,
 * just to check for thread safety of the implementation.
 * The different distance maps are not computed on the same graph,
 * since that operation does not seem to be thread safe.
 */
TEST_CASE ("Check dag shortest path calculation thread safety", "[shortest_path_parallel]") {
	// Initialise correct solution
	int vertex_count = 6;
	double distances_int[vertex_count];
	double inf = (std::numeric_limits<double>::max)();
	double eps = (std::numeric_limits<double>::epsilon)();
	double correct_distances[vertex_count][vertex_count] = { { 0, 1, 1, 2, 2, 2 },
								{ inf, 0, 1, 1, 2, 2 },
								{ inf, inf, 0, 1, 1, 1 },
								{ inf, inf, inf, 0, 1, 1 },
								{ inf, inf, inf, inf, 0, 1 },
								{ inf, inf, inf, inf, inf, 0 } };

	// Initialise graphs and distance maps
	// TODO maybe just copy g
	std::vector<gfcg::dag_t> graphs;
	std::vector<gfcg::distance_map_t> d_maps;
	graphs.reserve(vertex_count);
	d_maps.reserve(vertex_count);
	for (int j = 0; j < vertex_count; j++) {
		graphs.push_back(load_network(std::string(TEST_RES) + "/test_graph_26.xml").graph);
		d_maps.push_back(get(boost::vertex_distance, graphs.at(j)));
	}

	// Edge weights default to 0, so set them to 1 here
	// https://stackoverflow.com/questions/24366642/how-do-i-change-the-edge-weight-in-a-graph-using-the-boost-graph-library
	for (auto &g : graphs) {
		boost::property_map<gfcg::dag_t, boost::edge_weight_t>::type edge_weight_map = get(boost::edge_weight, g);
		boost::graph_traits<gfcg::dag_t>::edge_iterator e_it, e_end;
		for (std::tie(e_it, e_end) = boost::edges(g); e_it != e_end; ++e_it) {
			edge_weight_map[*e_it] = 1;
		}
	}

	// Asynchronously compute distance maps (on different graphs)
	std::vector<std::future<void> > futures;
	futures.reserve(vertex_count);
	for (int j = 0; j < vertex_count; j++) {
		// we can pass the objects explicitly by reference because we
		// can be sure the lvalues outlive the the thread
		// also we need to wrap the function in a lambda to make it take references
		futures.push_back((std::async(
					[](gfcg::dag_t &g, gfcg::distance_map_t &d_map, int s)
					{dag_shortest_paths(g, s, distance_map(d_map));},
					std::ref(graphs.at(j)), std::ref(d_maps.at(j)), j)));
	}

	// Check against precomputed solution
	for (int j = 0; j < vertex_count; j++) {
		// block for completion
		futures.at(j).get();
		for (int i = 0; i < vertex_count; i++) {
			REQUIRE (std::abs(d_maps.at(j)[i] - correct_distances[j][i]) < eps);
		}
	}
}
