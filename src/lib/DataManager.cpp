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

#include "DataManager.hpp"

std::vector<Constraint>& DataManager::get_constraints() {
	const std::lock_guard<std::mutex> lock(constraints_mutex_);
	return constraints_;
}

std::unordered_map<int, Network>& DataManager::get_networks() {
	const std::lock_guard<std::mutex> lock(networks_mutex_);
	return networks_;
}
std::unordered_map<Arc, std::vector<std::pair<SCIP_CONS *, double> > >& DataManager::get_arc_to_scip_constraints_map() {
	const std::lock_guard<std::mutex> lock(arc_to_scip_constraints_mutex_);
	return arc_to_scip_constraints_;
}

Network& DataManager::get_network(const int group) {
	auto res = networks_.find(group);
	if (res == networks_.end()) ABORT_F("Unable to retrieve network with id %i", group);
	auto& net = res->second;
	return net;
}

Network& DataManager::get_network(const Arc& arc) {
	auto res_group = arc_to_network_.find(arc.source.netzwerk);
	if (res_group == arc_to_network_.end()) ABORT_F("Unable to retrieve network for arc %s", arc.to_string().c_str());
	auto res = networks_.find(res_group->second);
	if (res == networks_.end()) ABORT_F("Unable to retrieve network by id %i", res_group->second);
	auto& net = res->second;
	return net;
}

const boost::optional<const std::vector<DataManager::constraint_data_t>& > DataManager::get_constraints_of_arc(const Arc& arc) const {
	auto res = arc_to_scip_constraints_.find(arc);
	return (res != arc_to_scip_constraints_.end()) ?
		res->second :
		boost::optional<const std::vector<DataManager::constraint_data_t>& >{};
}

const boost::optional<const Path&> DataManager::get_path(const std::string& varname) const {
	auto res = generated_var_to_path_.find(varname);
	return (res != generated_var_to_path_.end()) ?
		res->second :
		boost::optional<const Path&>{};
}

void DataManager::add_constraint(Constraint cons) {
	const std::lock_guard<std::mutex> lock(constraints_mutex_);
	constraints_.push_back(cons);
}

void DataManager::add_network(int group, Network net) {
	const std::lock_guard<std::mutex> lock(networks_mutex_);
	networks_.insert(std::make_pair(group, net));
}

void DataManager::add_scip_constraint_to_arc(const Arc& arc, SCIP_CONS * scip_cons, double coeff) {
	const std::lock_guard<std::mutex> lock(arc_to_scip_constraints_mutex_);
	auto res = arc_to_scip_constraints_.find(arc);
	if (res == arc_to_scip_constraints_.end()) {
		std::vector<std::pair<SCIP_CONS *, double>> cons_of_arc;
		cons_of_arc.push_back(std::make_pair(scip_cons, coeff));
		arc_to_scip_constraints_.insert(std::make_pair(arc, cons_of_arc));
	} else {
		res->second.push_back(std::make_pair(scip_cons, coeff));
	}
}

void DataManager::add_arcs_of_network(const int group, std::vector<Arc> arcs) {
	const std::lock_guard<std::mutex> lock(arc_to_network_mutex_);
	for (const auto& arc : arcs) {
		arc_to_network_.insert(std::make_pair(arc.source.netzwerk, group));
	}
}

void DataManager::add_var_path_pair(std::string varname, Path path) {
	const std::lock_guard<std::mutex> lock(generated_var_to_path_mutex_);
	generated_var_to_path_.insert(std::make_pair(varname, path));
}
