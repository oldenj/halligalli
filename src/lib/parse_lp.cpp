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

#include "parse_lp.hpp"

#include <vector>

#include "loguru.hpp"

#include "Node.hpp"


// can be either negX with X a number or just a number
// negX -> -X
int parse_negx(const std::string &index) {
	std::string prefix = "neg";

	try {
		auto pos_prefix = index.find(prefix);
		if (pos_prefix == std::string::npos) {
			return std::stoi(index);
		} else {
			return -std::stoi(index.substr(pos_prefix + prefix.length()));
		}
	} catch (...) { ABORT_F("Error parsing lp variable: invalid index (expecting a natural number or negX): %s", index.c_str()); }
}

bool parse_bool(const std::string &index) {
       if (index.compare("false") == 0) {
               return false;
       } else if (index.compare("true") == 0) {
               return true;
       } else { ABORT_F("Error parsing lp variable: invalid index (expecting true or false): %s", index.c_str()); }
}

Node parse_indices(const std::vector<std::string>& indices) {
	Node node = {
		std::stoi(indices.at(0)), // laufbahngruppe
		std::stoi(indices.at(1)), // laufbahn
		parse_negx(indices.at(2)), // dienstgrad
		parse_negx(indices.at(3)), // zeitscheibe
		std::stoi(indices.at(4)), // status
		parse_bool(indices.at(5)), // ausbildung (bool)
		std::stoi(indices.at(6)) // netzwerk
	};
	return node;
}

boost::optional<Arc> parse_lp_var(const std::string &var_name) {
	std::string prefix = "y(";

	// short circuit if we don't have a y-var or malformed y-var
	auto pos_source = var_name.find(prefix);
	if (pos_source == std::string::npos) return boost::optional<Arc>{};
	pos_source += prefix.length();

	std::string node_delimiter = ",";
	auto pos_target = var_name.find(node_delimiter);
	if (pos_target == std::string::npos) ABORT_F("Error parsing lp variable, seems to be malformed: %s", var_name.c_str());
	pos_target += node_delimiter.length();
	auto target_prefix_length = pos_target;

	std::string index_delimiter = "|";
	std::string suffix = ")";
	std::vector<std::string> indices_source;
	std::vector<std::string> indices_target;

	// this is a kind of double fence post problem, this QND solution seems good enough
	int pos_source_next = var_name.find(index_delimiter, pos_source);
	int pos_target_next = var_name.find(index_delimiter, pos_target);
	for (int i = 0; i < 5; i++) {
		indices_source.push_back(var_name.substr(pos_source, pos_source_next - pos_source));
		pos_source = pos_source_next + index_delimiter.length();
		pos_source_next = var_name.find(index_delimiter, pos_source);

		indices_target.push_back(var_name.substr(pos_target, pos_target_next - pos_target));
		pos_target = pos_target_next + index_delimiter.length();
		pos_target_next = var_name.find(index_delimiter, pos_target);
	}
	indices_source.push_back(var_name.substr(pos_source, pos_source_next - pos_source));
	pos_source = pos_source_next + index_delimiter.length();
	pos_source_next = var_name.find(node_delimiter, pos_source);

	indices_target.push_back(var_name.substr(pos_target, pos_target_next - pos_target));
	pos_target = pos_target_next + index_delimiter.length();
	pos_target_next = var_name.find(suffix, pos_target);

	indices_source.push_back(var_name.substr(pos_source, pos_source_next - pos_source));
	indices_target.push_back(var_name.substr(pos_target, pos_target_next - pos_target));

	try {
		Arc arc = {parse_indices(indices_source), parse_indices(indices_target)};
		return arc;
	} catch (...) { ABORT_F("Error parsing lp variable, seems to be malformed: %s", var_name.c_str()); }
}
