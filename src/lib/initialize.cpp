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

#include "initialize.hpp"

#include <assert.h>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <numeric>
#include <vector>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include "loguru.hpp"
#include <objscip/objscipdefplugins.h>

#include "Arc.hpp"
#include "Network.hpp"
#include "parse_lp.hpp"

void generate_networks(SCIP * scip, DataManager& data_manager, boost::asio::thread_pool& tpool, std::string graphs_location) {
	LOG_SCOPE_F(1, "Starting to parse graph XML files (generate_networks())");
	std::vector<std::future<void>> network_read_futures;
	bool network_found = false;
	for (const auto &entry : std::filesystem::directory_iterator(graphs_location)) {
		if (entry.path().extension().compare(".xml") == 0) {
			auto task = std::make_shared<std::packaged_task<void()>> (std::bind([&](std::string path) {
				LOG_F(2, "Parsing graph: \'%s\'", path.c_str());
				std::vector<Arc> arc_list;
				Network net(path, arc_list);
				int net_group = net.get_group();
				data_manager.add_network(net_group, std::move(net));
				data_manager.add_arcs_of_network(net_group, arc_list);
			}, std::move(entry.path())));
			network_read_futures.push_back(std::move(task->get_future()));
			boost::asio::post(tpool, std::bind(&std::packaged_task<void()>::operator(), task));
			network_found = true;
		}
	}
	for (auto& future : network_read_futures) future.wait();
	if (!network_found) ABORT_F("Did not find any XML files at %s", graphs_location.c_str());
}

void generate_maps(SCIP * scip, DataManager& data_manager, std::vector<std::string> unneeded_constraints_substrings) {
	SCIP_CONS** conss = SCIPgetConss(scip);
	int n_conss = SCIPgetNConss(scip);

	LOG_SCOPE_F(1, "Starting to preprocess LP and to generate mappings (generate_maps())");

	for (int i = 0; i < n_conss; i++) {
		// prune unneeded constraints by substrings of their name. substrings are defined in the config
		std::string cons_name(SCIPconsGetName(conss[i]));
		//auto fn = [](std::string a, std::string b) { return a.find(b) != std::string::npos; };
		//bool test = std::accumulate(unneeded_constraints_substrings.begin(), unneeded_constraints_substrings.end(), fn, bool.operator||);
		bool deleted = false;
		for (const auto& substring : unneeded_constraints_substrings) {
			if (cons_name.find(substring) != std::string::npos) {
				LOG_F(3, "Deleting constraint: %s", cons_name.c_str());
				if (SCIPdelCons(scip, conss[i]) != SCIP_OKAY) ABORT_F("Failed to delete constraint (%s)", cons_name.c_str());
				deleted = true;
			}
			if (deleted) break;
		}
		if (deleted) continue;

		// create vector of arcs that have non-zero coefficient in this constraint
		Constraint cons;
		cons.scip_constraint = conss[i];
		std::vector<Arc> arcs;

		bool is_pricing_constraint = false;
		SCIP_Bool success;

		int n_vars;
		if (SCIPgetConsNVars(scip, conss[i], &n_vars, &success) != SCIP_OKAY ) ABORT_F("Failed to retrieve variable count (%s)", cons_name.c_str());
		assert(success);

		SCIP_VAR * vars[n_vars];
		if (SCIPgetConsVars(scip, conss[i], vars, n_vars, &success) != SCIP_OKAY ) ABORT_F("Failed to retrieve variables (%s)", cons_name.c_str());
		assert(success);

		SCIP_Real coefficients[n_vars];
		if (SCIPgetConsVals(scip, conss[i], coefficients, n_vars, &success) != SCIP_OKAY ) ABORT_F("Failed to retrieve coefficients (%s)", cons_name.c_str());
		assert(success);

		for (int j = 0; j < n_vars; j++) {
			std::string var_name(SCIPvarGetName(vars[j]));
			auto arc_option = parse_lp_var(var_name);
			if (!arc_option) continue;
			auto arc = *arc_option;
			is_pricing_constraint = true;
			double coefficient = (double) coefficients[j];
			cons.arcs.push_back(std::make_pair(arc, coefficient));

			// for the reverse map: add current constraint to the constraint list of this arc (y-var)
			data_manager.add_scip_constraint_to_arc(arc, conss[i], coefficient);

			LOG_F(3, "Deleting variable %s from constraint %s", var_name.c_str(), cons_name.c_str());
			if (SCIPdelCoefLinear(scip, conss[i], vars[j]) != SCIP_OKAY)
				ABORT_F("Unable to delete variable (%s) from constraint (%s)", var_name.c_str(), cons_name.c_str());
		}

		if (is_pricing_constraint) {
			// mark cons modifiable to enable it for pricing
			LOG_F(3, "Setting constraint modifiable: %s", cons_name.c_str());
			if (SCIPsetConsModifiable(scip, conss[i], true) != SCIP_OKAY) ABORT_F("Unable to mark constraint (%s) modifiable", cons_name.c_str());

			// push the cons->vars map into the global list after gathering all y-vars
			data_manager.add_constraint(cons);
		}
	}
}

void initialize_container(SCIP * scip, DataManager& data_manager, boost::asio::thread_pool& tpool,
		std::string graphs_location, std::vector<std::string> unneeded_constraints_substrings) {
	assert(scip != NULL);
	generate_networks(scip, data_manager, tpool, graphs_location);
	generate_maps(scip, data_manager, unneeded_constraints_substrings);
}
