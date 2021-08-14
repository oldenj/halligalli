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

#include "Node.hpp"

#include <tuple>

bool Node::operator<(const Node& rhs) const {
	return std::tie(laufbahngruppe, laufbahn, dienstgrad,
			zeitscheibe, status, ausbildung, netzwerk) <
		std::tie(rhs.laufbahngruppe, rhs.laufbahn, rhs.dienstgrad,
				rhs.zeitscheibe, rhs.status, rhs.ausbildung, rhs.netzwerk);
}

bool Node::operator==(const Node& rhs) const {
	return
		laufbahngruppe == rhs.laufbahngruppe &&
		laufbahn == rhs.laufbahn &&
		dienstgrad == rhs.dienstgrad &&
		zeitscheibe == rhs.zeitscheibe &&
		status == rhs.status &&
		ausbildung == rhs.ausbildung &&
		netzwerk == rhs.netzwerk;
}
