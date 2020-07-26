/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2020
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include <thread>
#include <algorithm>
#include <functional>

#include <tbb/parallel_for.h>

#include "LinearMath/btThreads.h"

class TaskSchedulerTBB : public btITaskScheduler {
public:
	TaskSchedulerTBB() : btITaskScheduler("IntelTBB") {}

	virtual int getMaxNumThreads() const override {
		return std::max(1u, std::thread::hardware_concurrency());
	}

	virtual int getNumThreads() const override {
		//Just assume TBB uses the max avialable - no actual way of getting
		//the number without using deprecated interfaces.
		return std::max(1u, std::thread::hardware_concurrency());
	}

	virtual void setNumThreads(int numThreads) override {}

	virtual void parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody& body) override {
		tbb::parallel_for(tbb::blocked_range<int>(iBegin, iEnd, 0), [&body](const tbb::blocked_range<int>& range) {
			body.forLoop(range.begin(), range.end());
		});
	}

	virtual btScalar parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody& body) override {
		//Not even going to bother here, it's just addition. Might need changing if constraint support is added.
		return body.sumLoop(iBegin, iEnd);
	}
};
