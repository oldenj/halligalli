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

#ifndef __OBJPRICERGFCG_HPP
#define __OBJPRICERGFCG_HPP

#include <string>
#include <vector>

#include <boost/asio/thread_pool.hpp>
#include <objscip/objscip.h>

#include "DataManager.hpp"

/** Provides callbacks for the SCIP framework in order to generate new variables after each solving iteration
 * This is the main component of the Halligalli GFCG library.
 * PRICERREDCOST or PRICERFARKAS is called back after each solving iteration, depending on whether the current reduced master problem was feasible or not.
 * Using the helper classes and the relations stored in the DataManager instance, the pricing problem is solved and new variables are generated and added to the problem instance.
 */
class ObjPricerGFCG : public scip::ObjPricer {
	public:
		ObjPricerGFCG(SCIP * scip, const std::string pricer_name, DataManager& data_manager, boost::asio::thread_pool& tpool);
		virtual ~ObjPricerGFCG();
		virtual SCIP_DECL_PRICERINIT(scip_init);
		virtual SCIP_DECL_PRICERREDCOST(scip_redcost);
		virtual SCIP_DECL_PRICERFARKAS(scip_farkas);

	protected:
		SCIP_RETCODE pricing(SCIP * scip, bool farkas);
		/** This function decides which paths are to be added as variables and should be implemented by derived classes, depending on the pricing strategy */
		virtual int shortest_path_cg(SCIP * scip) = 0;
		SCIP_RETCODE update_constraint_pointers(SCIP * scip);
		SCIP_RETCODE zero_arc_weights();
		SCIP_RETCODE set_arc_weights(SCIP * scip, bool farkas);
		SCIP_RETCODE add_variable(SCIP * scip, SCIP_VAR * var, const std::vector<DataManager::constraint_data_t>& constraints_data, std::mutex& scip_mutex);
		SCIP_RETCODE generate_columns(SCIP * scip, Path path, std::mutex& scip_mutex);

		int n_generated_paths_;
		int n_iterations_;
		DataManager& data_manager_;
		boost::asio::thread_pool& tpool_;
};

#endif
