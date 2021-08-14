#include "Path.hpp"

bool Path::operator<(const Path& rhs) const {
	return length < rhs.length;
};
