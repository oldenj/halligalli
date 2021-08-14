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

#include "FullPricer.hpp"

#include <future>

#include <boost/asio.hpp>

FullPricer::FullPricer(SCIP * scip, const std::string pricer_name, DataManager& data_manager, boost::asio::thread_pool& tpool) :
	ObjPricerGFCG(scip, pricer_name, data_manager, tpool) {}

int FullPricer::shortest_path_cg(SCIP * scip) {
	std::mutex counter_mutex;
	int n_iteration_generated_paths = 0;

	DLOG_F(1, "Starting shortest path ...");
	std::vector<std::future<SCIP_RETCODE>> add_var_futures;
	std::mutex scip_mutex;

	for (auto &net : data_manager_.get_networks()) {
        	auto task = std::make_shared<std::packaged_task<SCIP_RETCODE()>> ([&]() {
        		auto path = net.second.shortest_path();
			if (SCIPisNegative(scip, path.length)) {
				LOG_SCOPE_F(2, "Found path with negative length (%f). Adding as variable.", path.length);

				generate_columns(scip, path, scip_mutex);

				counter_mutex.lock();
				n_iteration_generated_paths++;
				n_generated_paths_++;
				counter_mutex.unlock();
				return SCIP_OKAY;
			}
			return SCIP_OKAY;
		});
		add_var_futures.push_back(std::move(task->get_future()));
		boost::asio::post(tpool_, std::bind(&std::packaged_task<SCIP_RETCODE()>::operator(), task));
	}
	for (auto& future : add_var_futures)
		if (future.get() != SCIP_OKAY) ABORT_F("SCIP reported an error during the variable generation routine");

	LOG_F(1, "Completed pricing, generated %d new paths/variables", n_iteration_generated_paths);
	return n_iteration_generated_paths;
}
