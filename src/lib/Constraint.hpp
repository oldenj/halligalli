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

#ifndef __CONSTRAINT_HPP
#define __CONSTRAINT_HPP

#include <vector>
#include <utility>

#include <objscip/objscip.h>

#include <Arc.hpp>

/** Represents a constraint relevant to the pricing problem of the problem, i.e. it contained a y-var in the original LP.
 * Each constraint stores a vector of arcs (y-variables in the LP) that were part of the constraint in the original LP, including the coefficients.
 * This is needed to <br>
 * --- a) set this constraints dual price as edge weights on the correct network arcs <br>
 * --- b) add back generated variables to the correct constraints, with the correct coefficients <br>
 */
struct Constraint {
	SCIP_CONS * scip_constraint;
	std::vector<std::pair<Arc, double>> arcs;
};

#endif
