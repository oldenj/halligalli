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

#include "KShortestPricer.hpp"

#include <assert.h>
#include <future>

#include <boost/asio.hpp>
#include "loguru.hpp"

#include "Path.hpp"

KShortestPricer::KShortestPricer(SCIP * scip, const std::string pricer_name, DataManager& data_manager, boost::asio::thread_pool& tpool, int k) :
	ObjPricerGFCG(scip, pricer_name, data_manager, tpool),
	k_(k) {
	assert(k_ > 0);
}

int KShortestPricer::shortest_path_cg(SCIP * scip) {
	std::mutex scip_mutex;
	std::mutex counter_mutex;
	int iteration_generated_paths = 0;

	DLOG_F(1, "Starting shortest path ...");
	std::vector<std::future<Path>> path_futures;
	for (auto &net : data_manager_.get_networks()) {
		auto task = std::make_shared<std::packaged_task<Path()>> (std::bind(&Network::shortest_path, std::ref(net.second)));
		path_futures.push_back(std::move(task->get_future()));
		boost::asio::post(tpool_, std::bind(&std::packaged_task<Path()>::operator(), task));
	}
	std::vector<Path> negative_length_paths;
	for (auto& future : path_futures) {
		auto path = future.get();
		if (SCIPisNegative(scip, path.length)) negative_length_paths.push_back(std::move(path));
	}

	DLOG_F(1, "Apply pricing strategy ...");

	if (k_ < negative_length_paths.size()) {
		DLOG_F(3, "Sort ...");
		std::sort(negative_length_paths.begin(), negative_length_paths.end());
		DLOG_F(3, "Erase ...");
		negative_length_paths.erase(negative_length_paths.begin() + k_, negative_length_paths.end());
	}

	DLOG_F(1, "Adding new variables ...");
	std::vector<std::future<SCIP_RETCODE>> add_var_futures;
	for (const auto& path : negative_length_paths) {
		auto net = data_manager_.get_network(path.network_group);

		auto task = std::make_shared<std::packaged_task<SCIP_RETCODE()>> ([&]() {
			// add path as variable to master lp if it satisfies the condition
			if (!SCIPisNegative(scip, path.length)) return SCIP_OKAY;
			LOG_SCOPE_F(2, "Found path with negative length (%f). Adding as variable.", path.length);

			generate_columns(scip, path, scip_mutex);

			counter_mutex.lock();
			iteration_generated_paths++;
			n_generated_paths_++;
			counter_mutex.unlock();
			return SCIP_OKAY;
		});
		add_var_futures.push_back(std::move(task->get_future()));
		boost::asio::post(tpool_, std::bind(&std::packaged_task<SCIP_RETCODE()>::operator(), task));
	}
	for (auto& future : add_var_futures)
		if (future.get() != SCIP_OKAY) ABORT_F("SCIP reported an error during the variable generation routine");

	return iteration_generated_paths;
}
