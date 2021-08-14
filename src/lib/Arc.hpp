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

#ifndef __ARC_HPP
#define __ARC_HPP

#include <iostream>
#include <string>

#include "Node.hpp"

/** The struct representing an arc of the problem, appearing as y-variables in the LP and as nodes in the networks */
struct Arc {
	bool operator<(const Arc& rhs) const;
	bool operator==(const Arc& rhs) const;
	std::string to_string() const;
	std::ostream& operator<<(std::ostream& stream) const;

	Node source;
	Node target;
};

// Make the struct hashable to use it as a key in unordered maps
namespace std {
    template<> struct hash<Arc> {
        std::size_t operator()(const Arc& arc) const noexcept {
            std::size_t h1 = std::hash<Node>{}(arc.source);
            std::size_t h2 = std::hash<Node>{}(arc.target);
            return h1 ^ (h2 << 1);
        }
    };
}

#endif
