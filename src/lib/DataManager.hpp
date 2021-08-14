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

#ifndef __DATAMANAGER_HPP
#define __DATAMANAGER_HPP

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/optional.hpp>
#include <objscip/objscip.h>

#include "Arc.hpp"
#include "Constraint.hpp"
#include "Network.hpp"
#include "Path.hpp"

/** Stores and provides all relations and data needed for the column generation procedure in a thread safe manner */
class DataManager {
	public:
		typedef std::pair<SCIP_CONS *, double> constraint_data_t;

		std::vector<Constraint>& get_constraints();
		std::unordered_map<int, Network>& get_networks();
		std::unordered_map<Arc, std::vector<constraint_data_t> >& get_arc_to_scip_constraints_map();
		Network& get_network(const int group);
		Network& get_network(const Arc& arc);
		/// Returns all constraints which contain this arc in the master lp
		const boost::optional<const std::vector<constraint_data_t>& > get_constraints_of_arc(const Arc& arc) const;
		/// Returns the path corresponding to a generated variable
		const boost::optional<const Path&> get_path(const std::string& varname) const;

		void add_constraint(Constraint cons);
		void add_network(int group, Network net);
		/// Stores information about an arc being in a constraint of the master LP, including the coefficient
		void add_scip_constraint_to_arc(const Arc& arc, SCIP_CONS * scip_cons, double coeff);
		/// Stores information about the arcs present in a network (represented by its unique group id)
		void add_arcs_of_network(const int group, std::vector<Arc> arcs);
		/// Stores the part-of relation of a path, i.e. list of arcs, to a generated variable. This is needed to map the LP solution back to its original variables
		void add_var_path_pair(std::string varname, Path path);

	private:
		// All constraints relevant to pricing (including a priced variable). Contains a list of arcs for each constraint.
		std::mutex constraints_mutex_;
		std::vector<Constraint> constraints_;
		// All networks that are part of the pricing problem. Networks contain an Edge<->Arc mapping
		std::mutex networks_mutex_;
		std::unordered_map<int, Network> networks_;
		// Maps an arc to all constraints it was part of, including the coefficient.
		std::mutex arc_to_scip_constraints_mutex_;
		std::unordered_map<Arc, std::vector<constraint_data_t> > arc_to_scip_constraints_;
		// Map an arc to the network group id it is included in
		std::mutex arc_to_network_mutex_;
		std::unordered_map<int, int> arc_to_network_;
		// Map a generated variable to the arcs it represents and the group id of the network it was generated from (for reporting)
		std::mutex generated_var_to_path_mutex_;
		std::unordered_map<std::string, Path> generated_var_to_path_;
};

#endif
