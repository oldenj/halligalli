#include <string>

#include <boost/graph/dag_shortest_paths.hpp>

#include "catch.hpp"
#include "types.hpp"
#include "read_xml.hpp"

TEST_CASE ("Check dag shortest path calculation", "[shortest_path]") {
	gfcg::dag_t g = load_network(std::string(TEST_RES) + "/test_graph_26.xml").graph;
	gfcg::distance_map_t distances = get(boost::vertex_distance, g);

	// This stuff is verified in test_read_xml
	const int vertex_count = 6;
	double distances_int[vertex_count];
	double inf = (std::numeric_limits<double>::max)();
	double correct_distances[vertex_count][vertex_count] = { { 0, 1, 1, 2, 2, 2 },
								{ inf, 0, 1, 1, 2, 2 },
								{ inf, inf, 0, 1, 1, 1 },
								{ inf, inf, inf, 0, 1, 1 },
								{ inf, inf, inf, inf, 0, 1 },
								{ inf, inf, inf, inf, inf, 0 } };

	// Edge weights default to 0, so set them to 1 here
	// https://stackoverflow.com/questions/24366642/how-do-i-change-the-edge-weight-in-a-graph-using-the-boost-graph-library
	boost::property_map<gfcg::dag_t, boost::edge_weight_t>::type edge_weight_map = get(boost::edge_weight, g);
	boost::graph_traits<gfcg::dag_t>::edge_iterator e_it, e_end;
	for (std::tie(e_it, e_end) = boost::edges(g); e_it != e_end; ++e_it) {
		edge_weight_map[*e_it] = 1;
	}

	/*
	 * use sections and the intermediary distances_int array to prevent a strange segfault
	 * which appears when using the REQUIRE macro in between accesses to the
	 * distances map
	 */
	for (int j = 0; j < vertex_count; j++) {
		SECTION (std::string("Starting node ") + std::to_string(j) + " (ext id " + std::to_string(j + 1) + ")") {
			// update distances
			dag_shortest_paths(g, j, distance_map(distances));
			for (int i = 0; i < vertex_count; i++)
				distances_int[i] = distances[i];
			for (int i = 0; i < vertex_count; i++)
				// TODO why does this work without epsilon comparison after we changed from int to double?
				REQUIRE (distances_int[i] == correct_distances[j][i]);
		}
	}
}
