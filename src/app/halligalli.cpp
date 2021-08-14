/**
 * @file The reference application making use of the halligalli library which implements a SCIP pricer to solve a "generalized flow column generation problem" (GFCG).
 * @author Jurek Olden (jurek.olden@in.tum.de)
 * @author Victor Oancea (oancea@in.tum.de)
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

#include <assert.h>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio/thread_pool.hpp>
#include <nlohmann/json.hpp>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

#include "DataManager.hpp"
#include "FullPricer.hpp"
#include "KShortestPricer.hpp"
#include "ObjPricerGFCG.hpp"
#include "initialize.hpp"
#include "loguru.hpp"
#include "reporting.hpp"

using json = nlohmann::json;

int exec_main(int argc, char** argv) {
	std::fstream config_stream("config.json", std::ios::in);
	json config;
	config_stream >> config;

	auto lp_path = config["lp"].get<std::string>();
	auto graphs_path = config["graphs"].get<std::string>();
	auto tcount = config["threadcount"].get<int>();
	auto pricing_strategy = config["pricing_strategy"].get<int>();

	if (lp_path.empty() || graphs_path.empty()) {
		if (argc != 3) {
			LOG_F(FATAL, "Provide lp and graphs paths in the config file or as arguments");
			return SCIP_NOPROBLEM;
		}
		lp_path = argv[1];
		graphs_path = argv[2];
	}

	// Initialize logging - the init and add_file calls leak memory, was already reported upstream
	loguru::init(argc, argv);
	loguru::g_stderr_verbosity = config["loglevel"].get<int>();
	for (const auto& logfile_config : config["logfiles"])
		loguru::add_file(logfile_config["path"].get<std::string>().c_str(), loguru::Truncate, logfile_config["level"].get<int>());

	static std::string pricer_name = "GFCG_Pricer";

	SCIP* scip = NULL;
	SCIP_CALL( SCIPcreate(&scip) );
	SCIP_CALL( SCIPincludeDefaultPlugins(scip) );

	// TODO should we turn off separation? profiling, recheck results against scip -f
	SCIP_CALL( SCIPsetIntParam(scip, "presolving/maxrestarts", 0) );
	SCIP_CALL( SCIPsetSeparating(scip, SCIP_PARAMSETTING_OFF, TRUE) );

	if (tcount <= 0) tcount = (std::thread::hardware_concurrency() > 0) ? std::thread::hardware_concurrency() : 1;
	boost::asio::thread_pool tpool(tcount);

	LOG_F(INFO, "Reading problem...");
	SCIP_CALL( SCIPreadProb(scip, lp_path.c_str(), NULL) );

	LOG_F(INFO, "Preprocessing...");
	DataManager data_manager;
	initialize_container(scip, data_manager, tpool, graphs_path.c_str(), config["unneeded_constraints_substrings"].get<std::vector<std::string>>());

	std::unique_ptr<ObjPricerGFCG> pricer;
	if (pricing_strategy > 0) {
		pricer_name.append("_KShortest");
		pricer = std::make_unique<KShortestPricer>(scip, pricer_name, data_manager, tpool, pricing_strategy);
	} else {
		pricer_name.append("_Full");
		pricer = std::make_unique<FullPricer>(scip, pricer_name, data_manager, tpool);
	}
	assert(pricer != nullptr);

	SCIP_CALL( SCIPincludeObjPricer(scip, pricer.release(), true) );
	SCIP_CALL( SCIPactivatePricer(scip, SCIPfindPricer(scip, pricer_name.c_str())) );

	LOG_F(INFO, "Solving...");
	SCIP_CALL( SCIPsolve(scip) );

	LOG_F(INFO, "Reporting...");
	if (config["report_results_scip"].get<bool>()) SCIP_CALL( SCIPprintBestSol(scip, NULL, FALSE) );
	if (config["report_results_csv"].get<bool>()) report_results_csv(scip, data_manager);

	LOG_F(INFO, "Freeing SCIP object...");
	SCIP_CALL( SCIPfree(&scip) );
	BMScheckEmptyMemory();

	LOG_F(INFO, "Terminated with success");
	return SCIP_OKAY;
}

int main(int argc, char** argv) {
	return exec_main(argc, argv) != SCIP_OKAY ? 1 : 0;
}
