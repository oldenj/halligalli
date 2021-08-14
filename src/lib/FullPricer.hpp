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

#ifndef __FULLPRICER_HPP
#define __FULLPRICER_HPP

#include <string>
#include <vector>

#include <boost/asio/thread_pool.hpp>
#include <objscip/objscip.h>

#include "ObjPricerGFCG.hpp"
#include "DataManager.hpp"

/** Derived pricer class that adds all negative-length paths as variables */
class FullPricer : public ObjPricerGFCG {
	public:
		FullPricer(SCIP * scip, const std::string pricer_name, DataManager& data_manager, boost::asio::thread_pool& tpool);

	private:
		int shortest_path_cg(SCIP * scip);
};

#endif
