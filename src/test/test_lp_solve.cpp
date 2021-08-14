#include <iostream>
#include <vector>

#include "catch.hpp"
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

#include "pricer_gfcg.hpp"

TEST_CASE("Check a complete LP solve", "[lp_solve]") {
	auto lp_file_path = std::string(TEST_RES) + "/example_problem.lp";
	auto graphs_path = std::string(TEST_RES) + "/example_problem_graphs/";

   	static const char* c_pricer_name = "GFCG_Pricer";

	SCIP * scip = NULL;
	SCIP_CALL_ABORT( SCIPcreate(&scip) );

	SCIP_CALL_ABORT( SCIPincludeDefaultPlugins(scip) );

	// disable presolving restarts for column generation
	SCIP_CALL_ABORT( SCIPsetIntParam(scip, "presolving/maxrestarts", 0) );
	// disable output for tests
	SCIP_CALL_ABORT( SCIPsetIntParam(scip, "display/verblevel", 0) );

	SCIP_CALL_ABORT( SCIPreadProb(scip, lp_file_path.c_str(), NULL) );

   	// include pricer
	gfcg::Path_map path_map;
	ObjPricerGFCG* pricer_ptr = new ObjPricerGFCG(scip, c_pricer_name, graphs_path.c_str(), path_map);
   	SCIP_CALL_ABORT( SCIPincludeObjPricer(scip, pricer_ptr, true) );

	// activate pricer
	SCIP_CALL_ABORT( SCIPactivatePricer(scip, SCIPfindPricer(scip, c_pricer_name)) );

	SCIP_CALL_ABORT( SCIPsolve(scip) );

	// check solution
	auto eps = std::numeric_limits<SCIP_Real>::epsilon();

	int nvars = SCIPgetNVars(scip);
	SCIP_VAR ** vars = SCIPgetVars(scip);
	std::vector<SCIP_Real> vals;
	vals.reserve(nvars);
	SCIP_SOL * best_sol = SCIPgetBestSol(scip);
	SCIP_CALL_ABORT( SCIPgetSolVals(scip, best_sol, nvars, vars, vals.data()) );

	for (int i = 0; i < nvars; i++) {
		std::string varname = SCIPvarGetName(vars[i]);
		if (varname.compare("t_y(4,6)") == 0)
			REQUIRE(std::abs(vals[i] - 4) < eps);
		if (varname.compare("t_y(1,2)") == 0)
			REQUIRE(std::abs(vals[i] - 1) < eps);
		if (varname.compare("t_y(3,4)") == 0)
			REQUIRE(std::abs(vals[i] - 3) < eps);
		if (varname.compare("p_0") == 0)
			REQUIRE(std::abs(vals[i] - 3) < eps);
		if (varname.compare("p_1") == 0)
			REQUIRE(std::abs(vals[i] - 0) < eps);
		if (varname.compare("p_2") == 0)
			REQUIRE(std::abs(vals[i] - 1) < eps);
	}

	// TODO check results.csv

	SCIP_CALL_ABORT( SCIPfree(&scip) );

	BMScheckEmptyMemory();
}
