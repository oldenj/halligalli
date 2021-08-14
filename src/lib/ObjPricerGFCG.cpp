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

#include "ObjPricerGFCG.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <memory>

#include <boost/asio.hpp>
#include "loguru.hpp"
#include <objscip/objscipdefplugins.h>


ObjPricerGFCG::ObjPricerGFCG(SCIP * scip, const std::string pricer_name, DataManager& data_manager, boost::asio::thread_pool& tpool) :
	ObjPricer(scip, pricer_name.c_str(), "solve pricing problem by leveraging the graph structure of the lp", 0, TRUE),
	n_generated_paths_(0),
	n_iterations_(0),
	data_manager_(data_manager),
	tpool_(tpool) {
		assert(scip != NULL);
}

ObjPricerGFCG::~ObjPricerGFCG() {}

/** Callback on initialization of the pricer
 * In this step, all pointers to original constraints in our problem data
 * need to be reassigned to point to the new transformed constraints
 * after SCIP automatically adjusted them.
 */
SCIP_DECL_PRICERINIT(ObjPricerGFCG::scip_init) {
	LOG_F(1, "Initializing pricer");
	assert(scip != NULL);
	SCIP_CALL( update_constraint_pointers(scip) );
	return SCIP_OKAY;
}

/** Pricing callback if the current problem instance was feasible in the last solving iteration */
SCIP_DECL_PRICERREDCOST(ObjPricerGFCG::scip_redcost) {
	/*
	 * From the SCIP documentation:
	 * "In the usual case that the pricer either adds a new variable or ensures that there are no further variables with negative dual feasibility,
	 * the result pointer should be set to SCIP_SUCCESS.
	 * Only if the pricer aborts pricing without creating a new variable,
	 * but there might exist additional variables with negative dual feasibility,
	 * the result pointer should be set to SCIP_DIDNOTRUN."
	 *
	 * Since we can guarantee either a new variable or optimality,
	 * we can fix the pointer to SCIP_SUCCESS.
	 */
	(*result) = SCIP_SUCCESS;

	return pricing(scip, false);
}

/** Pricing callback if the current problem instance was infeasible in the last solving iteration */
SCIP_DECL_PRICERFARKAS(ObjPricerGFCG::scip_farkas) {
	/*
	 * From the SCIP documentation:
	 * "In the usual case that the pricer either adds a new variable or ensures that there are no further variables with negative dual feasibility,
	 * the result pointer should be set to SCIP_SUCCESS.
	 * Only if the pricer aborts pricing without creating a new variable,
	 * but there might exist additional variables with negative dual feasibility,
	 * the result pointer should be set to SCIP_DIDNOTRUN."
	 *
	 * Since we can guarantee either a new variable or optimality,
	 * we can fix the pointer to SCIP_SUCCESS.
	 */
	(*result) = SCIP_SUCCESS;

	return pricing(scip, true);
}

SCIP_RETCODE ObjPricerGFCG::zero_arc_weights() {
	LOG_F(2, "Resetting all edge weights ...");
	std::vector<std::future<void>> reset_weight_futures;
	for (auto &net : data_manager_.get_networks()) {
		auto task = std::make_shared<std::packaged_task<void()>> (std::bind(&Network::reset_edge_weights, std::ref(net.second)));
		reset_weight_futures.push_back(std::move(task->get_future()));
		boost::asio::post(tpool_, std::bind(&std::packaged_task<void()>::operator(), task));
	}
	for (const auto& future : reset_weight_futures) future.wait();
	return SCIP_OKAY;
}

/** Get dual prices for each relevant constraint and set weights according to the corresponding arcs
 * Let C be a constraint of the original LP, and y_1,...,y_n the y-variables of this constraint with coefficients a_1,...,a_n.
 * Then there exist corresponding arcs e_1,...,e_n in some network.
 * We need to add, for each i, a_i * p, where p is the dual price of C, to the edge weight of e_i.
 * Since a y-variable y' might have been present in multiple constraints of the original problem, we sum up the contributions of each constraint.
 * */
SCIP_RETCODE ObjPricerGFCG::set_arc_weights(SCIP * scip, bool farkas) {
	LOG_F(2, "Getting dual solutions/farkas coefficients and updating edge weights ...");
	std::vector<std::future<void>> set_weight_futures;
	for (const auto &cons : data_manager_.get_constraints()) {
		auto task = std::make_shared<std::packaged_task<void()>> ([&]() {
			double dual_val = !farkas ? -SCIPgetDualsolLinear(scip, cons.scip_constraint) : -SCIPgetDualfarkasLinear(scip, cons.scip_constraint);
			if (SCIPisZero(scip, dual_val)) return;
			for (const auto &arc_info : cons.arcs) {
				auto arc = arc_info.first;
				auto coefficient = arc_info.second;
				auto& net = data_manager_.get_network(arc);
				net.add_to_edge_weight(arc, coefficient * dual_val);
				DLOG_F(2, "Added weight %f to %s, new weight %d, group %i",
						coefficient * dual_val, arc.to_string().c_str(), net.get_edge_weight(arc), net.get_group());
			}
		});
		set_weight_futures.push_back(std::move(task->get_future()));
		boost::asio::post(tpool_, std::bind(&std::packaged_task<void()>::operator(), task));
	}
	for (const auto& future : set_weight_futures) future.wait();
	return SCIP_OKAY;
}

SCIP_RETCODE ObjPricerGFCG::add_variable(SCIP * scip, SCIP_VAR * var, const std::vector<DataManager::constraint_data_t>& constraints_data, std::mutex& scip_mutex) {
	for (auto &cons_info : constraints_data) {
		auto cons = cons_info.first;
		auto coefficient = cons_info.second;
		#ifndef NDEBUG
		std::string cons_name(SCIPconsGetName(cons));
		LOG_F(3, "Adding path variable (coefficient %f) to constraint %s", coefficient, cons_name.c_str());
		#endif
		scip_mutex.lock();
		assert( SCIPconsIsModifiable(cons) );
		SCIP_CALL( SCIPaddCoefLinear(scip, cons, var, coefficient) );
		scip_mutex.unlock();
	}
	return SCIP_OKAY;
}

/** Add a new variable to the problem instance
 * After finding a path of negative length, we may add this path to the problem instance as a variable.
 * For each arc e_i on the path, we can define a corresponding y-variable y_i.
 * We need to find all constraints C_i,1,...,C_i,n that included this y-variable y_i in the original master problem, with the help of the DataManager instance.
 * It's possible that there exists no such constraint C.
 * Then, we add the newly generated variable to each of these constraints with the original coefficient of the corresponding y-variable.
 * */
SCIP_RETCODE ObjPricerGFCG::generate_columns(SCIP * scip, Path path, std::mutex& scip_mutex) {
	std::string varname = "p_" + std::to_string(n_generated_paths_);
	SCIP_VAR * var = NULL;
	scip_mutex.lock();
	SCIP_CALL( SCIPcreateVarBasic(
				scip, &var, varname.c_str(),
				0, // lower bound
				SCIPinfinity(scip), // upper bound
				0, // objective
				SCIP_VARTYPE_CONTINUOUS) );
	SCIP_CALL( SCIPaddPricedVar(scip, var, 1.0) );
	scip_mutex.unlock();
	for (const auto &arc : path.arcs) {
		LOG_F(3, "Current edge (y-var): %s", arc.to_string().c_str());
		#ifndef NDEBUG
		LOG_SCOPE_F(3, "Finding containing constraints ...");
		#endif

		auto constraints_optional = data_manager_.get_constraints_of_arc(arc);
		if (!constraints_optional) continue;
		auto constraints = *constraints_optional;
		// it->second is the vector of constraints for current lp_arc
		add_variable(scip, var, constraints, scip_mutex);
	}

	// in order to know the flow over arcs (y-vars) given the solution, keep track of arcs belonging to the current variable
	data_manager_.add_var_path_pair(varname, path);
	SCIP_CALL( SCIPreleaseVar(scip, &var) );
	return SCIP_OKAY;
}

SCIP_RETCODE ObjPricerGFCG::pricing(SCIP * scip, bool farkas) {
	assert(scip != NULL);
	LOG_SCOPE_F(1, "Pricing starts (%s), iteration %d ... ", !farkas ? "redcost" : "farkas", n_iterations_);
	LOG_F(1, "Current primal bound: %f", SCIPgetPrimalbound(scip));
	zero_arc_weights();
	set_arc_weights(scip, farkas);
	int generated_paths = shortest_path_cg(scip);

	LOG_F(1, "Completed pricing, generated %d new paths/variables", generated_paths);
	n_iterations_ += 1;
	return SCIP_OKAY;
}

// TODO threading
SCIP_RETCODE ObjPricerGFCG::update_constraint_pointers(SCIP * scip) {
	LOG_F(3, "Updating constraint pointers after presolve");
	for (auto &cons : data_manager_.get_constraints()) {
		SCIP_CALL( SCIPgetTransformedCons(scip, cons.scip_constraint, &cons.scip_constraint) );
	}
	// iterate over all arc - constraints pairs
	for (auto &pair : data_manager_.get_arc_to_scip_constraints_map()) {
		// iterate over all constraints of the arc
		for (auto &cons_info : pair.second) {
			SCIP_CALL( SCIPgetTransformedCons(scip, cons_info.first, &cons_info.first) );
		}
	}
	return SCIP_OKAY;
}
