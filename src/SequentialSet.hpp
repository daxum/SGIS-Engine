/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018
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

#include <vector>
#include <unordered_map>
#include <memory>

//A restricted vector-like container used for having the remove/access times of sets
//while still retaining the sequential access speeds of vectors. According to benchmarks,
//though, it might just be best to use an unordered_set. getShared would need a replacement,
//however.
//Made specifically for use with smart pointers.
template<typename T>
class SequentialSet {
public:
	/**
	 * Sticks the thing into the map.
	 * @param object The thing to stick into the set.
	 */
	void insert(std::shared_ptr<T> object) {
		indexMap[object.get()] = vec.size();
		vec.push_back(object);
	}

	/**
	 * Removes the thing from the set.
	 * @param The thing to remove.
	 */
	void erase(std::shared_ptr<T> object) {
		//Check for existence.
		if (indexMap.count(object.get()) == 0) {
			return;
		}

		//Switch with last, update map, and remove.
		vec[indexMap[object.get()]] = vec.back();

		indexMap[vec.back().get()] = indexMap[object.get()];

		vec.pop_back();
		indexMap.erase(object.get());
	}

	/**
	 * Gets the smart pointer for the given pointer.
	 * @param The thing to get a smart pointer to, null if not present.
	 */
	std::shared_ptr<T> getShared(T* object) {
		if (indexMap.count(object)) {
			return vec[indexMap.at(object)];
		}

		return std::shared_ptr<T>();
	}

	/**
	 * Does exactly what it sounds like.
	 */
	size_t size() { return vec.size(); }

	typename std::vector<std::shared_ptr<T>>::iterator begin() {
		return vec.begin();
	}

	typename std::vector<std::shared_ptr<T>>::iterator end() {
		return vec.end();
	}

private:
	//Map of indices in the vector.
	std::unordered_map<T*, size_t> indexMap;
	//The vector that holds the objects.
	std::vector<std::shared_ptr<T>> vec;
};
