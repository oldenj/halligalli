#include <string>
#include <thread>
#include <vector>

#include <boost/asio/thread_pool.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <catch2/catch.hpp>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

#include "Arc.hpp"
#include "DataManager.hpp"
#include "Network.hpp"
#include "Node.hpp"
#include "initialize.hpp"

TEST_CASE("Test problem initialization and data storage", "[initialize]") {
	int tcount = (std::thread::hardware_concurrency() > 0) ? std::thread::hardware_concurrency() : 1;
	boost::asio::thread_pool tpool(tcount);

	auto lp_file_path = std::string(TEST_RES) + "/example_problem.lp";
	auto graphs_path = std::string(TEST_RES) + "/example_problem_graphs/";
	std::vector<std::string> unneeded_constraints_substrings = { "Flssbed" };

	// setup the problem and read lp file to test map generation
	SCIP * scip = NULL;
	SCIP_CALL_ABORT( SCIPcreate(&scip) );
	SCIP_CALL_ABORT( SCIPincludeDefaultPlugins(scip) );
	SCIP_CALL_ABORT( SCIPsetIntParam(scip, "display/verblevel", 0) );
	SCIP_CALL_ABORT( SCIPreadProb(scip, lp_file_path.c_str(), NULL) );

	DataManager data_manager;
	initialize_container(scip, data_manager, tpool, graphs_path, unneeded_constraints_substrings);

	// in the following checks we don't do epsilon comparisons since we only do integer arithmetics on double types
	SECTION("Check constraints") {
		auto constraints = data_manager.get_constraints();
		REQUIRE(constraints.size() == 3);
	}
	SECTION("Check network reference passing") {
		// arc from net group 1
		Node source = { 1, 1, 0, 0, 2, false, 1 };
		Node target = { 1, 1, 0, 0, 4, false, 1 };
		Arc arc = { source, target };

		Network& net = data_manager.get_network(1);
		double weight = net.get_edge_weight(arc);
		double new_weight = weight + 5;
		net.set_edge_weight(arc, new_weight);
		Network& net_new_ref_by_arc = data_manager.get_network(arc);
		Network& net_new_ref_by_group = data_manager.get_network(net.get_group());
		REQUIRE(net_new_ref_by_arc.get_edge_weight(arc) == new_weight);
		REQUIRE(net_new_ref_by_group.get_edge_weight(arc) == new_weight);
	}
	SECTION("Check group->network") {
		auto net1 = data_manager.get_network(1);
		auto net2 = data_manager.get_network(2);
		REQUIRE(net1.get_group() == 1);
		REQUIRE(net2.get_group() == 2);
	}
	SECTION("Check arc->network") {
		Node source_net1 = { 1, 1, 0, 0, 2, false, 1 };
		Node target_net1 = { 1, 1, 0, 0, 4, false, 1 };
		Arc arc_net1 = { source_net1, target_net1 };

		Node source_net2 = { 1, 1, 0, 0, 3, false, 2 };
		Node target_net2 = { 1, 1, 0, 0, 4, false, 2 };
		Arc arc_net2 = { source_net2, target_net2 };

		REQUIRE(data_manager.get_network(arc_net1).get_group() == 1);
		REQUIRE(data_manager.get_network(arc_net2).get_group() == 2);
	}
	SECTION("Check arc->constraints") {
		Node source = { 1, 1, 0, 0, 3, false, 1 };
		Node target = { 1, 1, 0, 0, 4, false, 1 };
		Arc arc = { source, target };

		auto constraints_option = data_manager.get_constraints_of_arc(arc);
		REQUIRE(constraints_option->size() == 1);
		std::string cons_name(SCIPconsGetName(constraints_option->at(0).first));
		double coefficient = constraints_option->at(0).second;
		REQUIRE(cons_name.compare("beta3") == 0);
		REQUIRE(coefficient == -1);
	}
}
