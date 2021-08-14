#include "Arc.hpp"

#include <tuple>

/** Less operator such that arcs may be used as keys in associative containers
 * @return lexicographical ordering.
 */
bool Arc::operator<(const Arc& rhs) const {
	return std::tie(source, target) <
		std::tie(rhs.source, rhs.target);
}

/** Equality operator
 * @return true iff both source and target nodes of the operands are equal.
 */
bool Arc::operator==(const Arc& rhs) const {
	return (source == rhs.source) && (target == rhs.target);
}

/** Pretty print an arc
 * @return The arc in human readable form, as presented in LP files.
 */
std::string Arc::to_string() const {
	return "y("
		+ std::to_string(source.laufbahngruppe) + "|"
		+ std::to_string(source.laufbahn) + "|"
		+ std::to_string(source.dienstgrad) + "|"
		+ std::to_string(source.zeitscheibe) + "|"
		+ std::to_string(source.status) + "|"
		+ std::to_string(source.ausbildung) + "|"
		+ std::to_string(source.netzwerk) + ","
		+ std::to_string(target.laufbahngruppe) + "|"
		+ std::to_string(target.laufbahn) + "|"
		+ std::to_string(target.dienstgrad) + "|"
		+ std::to_string(target.zeitscheibe) + "|"
		+ std::to_string(target.status) + "|"
		+ std::to_string(target.ausbildung) + "|"
		+ std::to_string(target.netzwerk) + ")";
}

std::ostream& Arc::operator<<(std::ostream& stream) const {
	stream << to_string();

	return stream;
}
