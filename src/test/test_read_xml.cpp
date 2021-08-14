#include <string>

#include <boost/graph/adjacency_list.hpp>
#include <catch2/catch.hpp>

#include "Arc.hpp"
#include "Network.hpp"
#include "Node.hpp"

TEST_CASE("Load test JGraphT XML", "[load_xml]") {
	std::vector<Arc> arc_list;
	Network net = Network(std::string(TEST_RES) + "/test_graph_26.xml", arc_list);

	REQUIRE(net.get_group() == 26);
	REQUIRE(net.get_vertex_count() == 8);
	REQUIRE(net.get_edge_count() == 10);

	Node source = { 1, 1, 1, 0, 1, false, 1 };
	Node target = { 1, 1, 2, 1, 1, false, 2 };
	Arc arc = {source, target};
	REQUIRE(std::find(arc_list.begin(), arc_list.end(), arc) != arc_list.end());

	// TODO check source/target node
	// TODO check error handling (e.g. missing source node etc.)
}
