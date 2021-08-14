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

#ifndef __NODE_HPP
#define __NODE_HPP

#include <boost/container_hash/hash.hpp>

/**
 * @brief Represents a network node
 * @author Jurek Olden (jurek.olden@in.tum.de)
 */
struct Node {
	int laufbahngruppe;
	int laufbahn;
	int dienstgrad;
	int zeitscheibe;
	int status;
	bool ausbildung;
	int netzwerk;

	bool operator<(const Node& rhs) const;
	bool operator==(const Node& rhs) const;
};

namespace std {
	template<> struct hash<Node> {
		std::size_t operator()(const Node& node) const noexcept {
			std::size_t seed = 0;
			boost::hash_combine(seed, node.laufbahngruppe);
			boost::hash_combine(seed, node.laufbahn);
			boost::hash_combine(seed, node.dienstgrad);
			boost::hash_combine(seed, node.zeitscheibe);
			boost::hash_combine(seed, node.status);
			boost::hash_combine(seed, node.ausbildung);
			boost::hash_combine(seed, node.netzwerk);
			return seed;
		}
	};
}

#endif
