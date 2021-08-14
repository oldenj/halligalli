#include <string>

#include <boost/optional.hpp>
#include <catch2/catch.hpp>

#include "Arc.hpp"
#include "parse_lp.hpp"

TEST_CASE("Parse LP variable name", "[parse_lp_var]") {
	// TODO check malformed var names / var names that are no y-vars
	SECTION("First test") {
		std::string test_var_name = "y(UmP|TrD|16|40|3|false|3734,UmP|TrD|16|41|3|false|3734)";
		auto test_arc_option = parse_lp_var(test_var_name);
		if (!test_arc_option) REQUIRE(false);
		Arc test_arc = *test_arc_option;

		REQUIRE(test_arc.source.laufbahngruppe == 5);
		REQUIRE(test_arc.source.laufbahn == 1);
		REQUIRE(test_arc.source.dienstgrad == 16);
		REQUIRE(test_arc.source.zeitscheibe == 40);
		REQUIRE(test_arc.source.status == 3);
		REQUIRE(test_arc.source.ausbildung == false);
		REQUIRE(test_arc.source.netzwerk == 3734);

		REQUIRE(test_arc.target.laufbahngruppe == 5);
		REQUIRE(test_arc.target.laufbahn == 1);
		REQUIRE(test_arc.target.dienstgrad == 16);
		REQUIRE(test_arc.target.zeitscheibe == 41);
		REQUIRE(test_arc.target.status == 3);
		REQUIRE(test_arc.target.ausbildung == false);
		REQUIRE(test_arc.target.netzwerk == 3734);
	}
	SECTION("Second test") {
		std::string test_var_name = "y(Unbek|Unbek|neg2|neg1|3|false|3734,UmP|TrD|16|41|3|false|3734)";
		auto test_arc_option = parse_lp_var(test_var_name);
		if (!test_arc_option) REQUIRE(false);
		Arc test_arc = *test_arc_option;

		REQUIRE(test_arc.source.laufbahngruppe == 0);
		REQUIRE(test_arc.source.laufbahn == 0);
		REQUIRE(test_arc.source.dienstgrad == -2);
		REQUIRE(test_arc.source.zeitscheibe == -1);
		REQUIRE(test_arc.source.status == 3);
		REQUIRE(test_arc.source.ausbildung == false);
		REQUIRE(test_arc.source.netzwerk == 3734);

		REQUIRE(test_arc.target.laufbahngruppe == 5);
		REQUIRE(test_arc.target.laufbahn == 1);
		REQUIRE(test_arc.target.dienstgrad == 16);
		REQUIRE(test_arc.target.zeitscheibe == 41);
		REQUIRE(test_arc.target.status == 3);
		REQUIRE(test_arc.target.ausbildung == false);
		REQUIRE(test_arc.target.netzwerk == 3734);
	}
}
