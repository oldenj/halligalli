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

#ifndef __INITIALIZE_HPP
#define __INITIALIZE_HPP

#include <string>
#include <vector>

#include <boost/asio/thread_pool.hpp>
#include <objscip/objscip.h>

#include "DataManager.hpp"

void initialize_container(SCIP * scip, DataManager& data_manager, boost::asio::thread_pool& tpool, std::string graphs_location, std::vector<std::string> unneeded_constraints_substrings);

#endif
