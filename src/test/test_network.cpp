#include <string>

#include <boost/graph/adjacency_list.hpp>
#include <catch2/catch.hpp>

#include "Arc.hpp"
#include "Network.hpp"
#include "Node.hpp"
#include "Path.hpp"

TEST_CASE("Check Network class", "[network]") {
	std::vector<Arc> arc_list;
	std::vector<Arc> arc_list_shortest_path;
	Network net_readin = Network(std::string(TEST_RES) + "/test_graph_26.xml", arc_list);
	Network net_shortest_path = Network(std::string(TEST_RES) + "/test_graph_shortest_path_1.xml", arc_list_shortest_path);

	SECTION("Check correct readin of JGraphT XML") {
		Network net = net_readin;

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
	SECTION("Check graph features") {
		Network net = net_shortest_path;

		Node source = { 0, 0, -2, -1, 0, false, 1 };
		Node node_2 = { 0, 0, 0, 0, 2, false, 1 };
		Node node_3 = { 0, 0, 0, 0, 3, false, 1 };
		Node target = { 0, 0, 35, -1, 0, false, 1 };
		Arc a_s_2 = { source, node_2 };
		Arc a_s_3 = { source, node_3 };
		Arc a_2_t = { node_2, target };
		Arc a_3_t = { node_3, target };

		// TODO we can rely on clean ints here but maybe do epsilon checks anyway
		// shortest path arc list begins at the target node
		Path shortest_path;
		SECTION("Check shortest path computation") {
			shortest_path = net.shortest_path();
			REQUIRE(shortest_path.network_group == net.get_group());
			REQUIRE(shortest_path.length == 0);
		}
		SECTION("Check edge weight manipulation") {
			net.set_edge_weight(a_s_2, 5);
			REQUIRE(net.get_edge_weight(a_s_2) == 5);
			shortest_path = net.shortest_path();
			REQUIRE(shortest_path.length == 0);
			REQUIRE(shortest_path.arcs.at(1) == a_s_3);
			REQUIRE(shortest_path.arcs.at(0) == a_3_t);

			net.add_to_edge_weight(a_s_3, -1);
			shortest_path = net.shortest_path();
			REQUIRE(shortest_path.length == -1);
			REQUIRE(shortest_path.arcs.at(1) == a_s_3);
			REQUIRE(shortest_path.arcs.at(0) == a_3_t);

			net.add_to_edge_weight(a_s_3, -1);
			shortest_path = net.shortest_path();
			REQUIRE(shortest_path.length == -2);

			net.reset_edge_weights();
			shortest_path = net.shortest_path();
			REQUIRE(net.get_edge_weight(a_s_2) == 0);
			REQUIRE(net.get_edge_weight(a_s_3) == 0);
			REQUIRE(net.get_edge_weight(a_2_t) == 0);
			REQUIRE(net.get_edge_weight(a_3_t) == 0);
			REQUIRE(shortest_path.length == 0);

			net.add_to_edge_weight(a_s_2, -2);
			net.add_to_edge_weight(a_s_3, -3);
			net.add_to_edge_weight(a_s_2, -3);
			net.add_to_edge_weight(a_2_t, 2);
			net.add_to_edge_weight(a_3_t, -1);
			shortest_path = net.shortest_path();
			REQUIRE(shortest_path.length == -4);
			REQUIRE(shortest_path.arcs.at(1) == a_s_3);
			REQUIRE(shortest_path.arcs.at(0) == a_3_t);
		}
	}
}
