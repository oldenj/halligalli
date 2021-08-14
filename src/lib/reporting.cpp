#include "reporting.hpp"

#include <assert.h>
#include <iostream>
#include <fstream>

#include <boost/optional.hpp>

#include "Arc.hpp"

void write_path(std::ofstream &file, const Path& path, SCIP_Real val) {
	if (!(val > 0)) return;
	for (auto &arc : path.arcs) {
		auto s = arc.source;
		auto t = arc.target;

		file << s.laufbahngruppe << ","
			<< s.laufbahn << ","
			<< s.dienstgrad << ","
			<< s.zeitscheibe << ","
			<< s.status << ","
			<< s.ausbildung << ","
			<< s.netzwerk << ","
			<< t.laufbahngruppe << ","
			<< t.laufbahn << ","
			<< t.dienstgrad << ","
			<< t.zeitscheibe << ","
			<< t.status << ","
			<< t.ausbildung << ","
			<< t.netzwerk << ","
			<< path.network_group << ","
			<< val << "\n";
	}
}

void report_results_csv(SCIP * scip, DataManager& data_manager) {
	assert(scip != NULL);

	std::ofstream file;
	file.open("results.csv");

	file << "laufbahngruppe.von" << ","
		<< "laufbahn.von" << ","
		<< "dienstgrad.von" << ","
		<< "zeitscheibe.von" << ","
		<< "status.von" << ","
		<< "ausbildung.von" << ","
		<< "netzwerk.von" << ","
		<< "laufbahngruppe.zu" << ","
		<< "laufbahn.zu" << ","
		<< "dienstgrad.zu" << ","
		<< "zeitscheibe.zu" << ","
		<< "status.zu" << ","
		<< "ausbildung.zu" << ","
		<< "netzwerk.zu" << ","
		<< "gruppe" << ","
		<< "wert" << "\n";

	int nvars = SCIPgetNVars(scip);
	SCIP_VAR ** vars = SCIPgetVars(scip);
	std::vector<SCIP_Real> vals;
	vals.reserve(nvars);
	SCIP_SOL * best_sol = SCIPgetBestSol(scip);
	SCIP_CALL_ABORT( SCIPgetSolVals(scip, best_sol, nvars, vars, vals.data()) );

	for (int i = 0; i < nvars; i++) {
		std::string varname = SCIPvarGetName(vars[i]);
		// skip all variables other than p_n
		auto path_option = data_manager.get_path(varname);
		if (!path_option) continue;
		const auto& path = *path_option;
		write_path(file, path, vals[i]);
	}
	file.close();
}
