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

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "ExtraMath.hpp"

template<typename T>
struct Aabb {
	typedef glm::vec<3, T, glm::defaultp> vec_t;

	vec_t min;
	vec_t max;

	/**
	 * Constructs a default-initialized axis-aligned bounding box.
	 */
	Aabb() :
		min(0, 0, 0),
		max(0, 0, 0) {

	}

	/**
	 * Constructs an axis-aligned bounding box using the given maximum
	 * and minimum values. Care should be taken that maximum is actually
	 * greater than minimum!
	 * @param min The corner of the box closest to zero.
	 * @param max The corner of the box farthest from zero.
	 */
	Aabb(vec_t min, vec_t max) :
		min(min),
		max(max) {

	}

	/**
	 * Constructs an axis-aligned bounding box that contains the entirety of
	 * both box1 and box2.
	 * TODO: any number of sub-boxes?
	 * @param box1 The first box this one should contain.
	 * @param box2 The second box this one should contain.
	 */
	Aabb(const Aabb<T>& box1, const Aabb<T>& box2) :
		min(std::min(box1.min.x, box2.min.x), std::min(box1.min.y, box2.min.y), std::min(box1.min.z, box2.min.z)),
		max(std::max(box1.max.x, box2.max.x), std::max(box1.max.y, box2.max.y), std::max(box1.max.z, box2.max.z)) {

	}

	/**
	 * Checks whether this box intersects with the given one.
	 * Boxes that are just touching are not considered intersecting.
	 * @param other The box to check for intersection with.
	 * @return Whether the two boxes intersect.
	 */
	bool intersects(const Aabb<T>& other) const  {
		return min.x < other.max.x && max.x > other.min.x &&
			   min.y < other.max.y && max.y > other.min.y &&
			   min.z < other.max.z && max.z > other.min.z;
	}

	/**
	 * Checks whether this box contains the given one, whether the given
	 * box is a complete subset of this one.
	 * @param other The box to check.
	 * @return Whether this box completely encompasses the given one.
	 */
	bool contains(const Aabb<T>& other) const  {
		return min.x <= other.min.x && max.x >= other.max.x &&
			   min.y <= other.min.y && max.y >= other.max.y &&
			   min.z <= other.min.z && max.z >= other.max.z;
	}

	/**
	 * Checks whether this box forms a box with the other box - this
	 * happens when they don't intersect, two of their dimensions are
	 * the same, and they touch on the third dimension.
	 * @param other The box to check.
	 * @return whether the two boxes form a bigger box.
	 */
	bool formsBoxWith(const Aabb<T>& other) const  {
		//X touching
		if (max.y == other.max.y && max.z == other.max.z &&
			min.y == other.min.y && min.z == other.min.z &&
			((min.x == other.max.x) ^ (max.x == other.min.x))) {

			return true;
		}

		//Y touching
		if (max.x == other.max.x && max.z == other.max.z &&
			min.x == other.min.x && min.z == other.min.z &&
			((min.y == other.max.y) ^ (max.y == other.min.y))) {

			return true;
		}

		//Z touching
		if (max.x == other.max.x && max.y == other.max.y &&
			min.x == other.min.x && min.y == other.min.y &&
			((min.z == other.max.z) ^ (max.z == other.min.z))) {

			return true;
		}

		//Intersecting or disjoint
		return false;
	}

	/**
	 * Bisects a bounding box along the given axis.
	 * @param axis The axis to bisect along. Each increasing value
	 *     is the next dimension, so x=0, y=1, z=2, etc. This theoretically
	 *     supports bisecting n-dimensional bounding boxes, if glm could
	 *     handle that.
	 * @param offset The offset along the axis to bisect. If this outside
	 *     the min or max in the given dimension, this will create zero- or
	 *     negative-volume bounding boxes, which probably isn't desired.
	 * @return The two boxes formed by bisecting this one.
	 */
	std::array<Aabb<T>, 2> bisect(size_t axis, const T& offset) const {
		std::array<Aabb<T>, 2> out = {*this, *this};

		out.at(0).max[axis] = offset;
		out.at(1).min[axis] = offset;

		return out;
	}

	/**
	 * Removes the given bounding box's volume from this one, forming one or
	 * more bounding boxes, all of which are contained in this one and none
	 * of which intersect with minus.
	 * @param minus The box representing the area to remove.
	 * @return A vector of bounding boxes of maximal volume which meet the
	 *     above criteria.
	 */
	std::vector<Aabb<T>> subtract(const Aabb<T>& minus) const {
		std::vector<Aabb<T>> out(1, *this);

		//Bisect into 0-3 boxes along each axis, discard the box that fully intersects with minus
		for (size_t i = 0; i < 3; i++) {
			//Remove last added box for splitting
			Aabb<T> splitBox = out.back();
			out.pop_back();

			//If minimum value intersects
			if (minus.min[i] > splitBox.min[i] && minus.min[i] < splitBox.max[i]) {
				//Bisect
				std::array<Aabb<T>, 2> split = bisect(splitBox, i, minus.min[i]);

				//Below minus - can't intersect
				out.push_back(split.at(0));
				//Set split box to possibly intersecting box
				splitBox = split.at(1);
			}

			//If maximum value intersects
			if (minus.max[i] < splitBox.max[i] && minus.max[i] > splitBox.min[i]) {
				//Bisect
				std::array<Aabb<T>, 2> split = bisect(splitBox, i, minus.max[i]);

				//Above minus - can't intersect
				out.push_back(split.at(1));
				//Set box for next dimension to possibly intersecting box
				splitBox = split.at(0);
			}

			//Add back whatever's left
			out.push_back(splitBox);
		}

		//Last box added fully intersects minus - remove it unless boxes don't intersect at all
		if (intersects(minus)) {
			out.pop_back();
		}

		return out;
	}

	/**
	 * Calculates the center of the box.
	 * @return The center.
	 */
	vec_t getCenter() const  {
		return vec_t(
			(min.x + max.x) / 2.0f,
			(min.y + max.y) / 2.0f,
			(min.z + max.z) / 2.0f
		);
	}

	/**
	 * Calculates the volume of the box.
	 * @return The volume.
	 */
	T getVolume() const {
		return xLength() * yLength() * zLength();
	}

	/**
	 * The below three functions get the dimensions of the box.
	 * @return The lengths of the sides of the box.
	 */
	T xLength() const { return max.x - min.x; }
	T yLength() const { return max.y - min.y; }
	T zLength() const { return max.z - min.z; }

	/**
	 * Translates the box dist units.
	 * @param dist The distance to translate.
	 */
	void translate(const vec_t& dist) {
		min += dist; max += dist;
	}

	/**
	 * Scales the box by the given amount.
	 * @param amount The amount to scale the box by.
	 */
	void scale(const vec_t& amount) {
		vec_t center = getCenter();

		vec_t diff = (max - center) * amount - (max - center);

		min -= diff;
		max += diff;
	}

	/**
	 * Scales all three sides of the box at once.
	 * @param amount The amount to scale the sides by.
	 */
	void scaleAll(const T& amount) { scale({amount, amount, amount}); }

	/**
	 * Converts the box to a printable string.
	 * @return A string representation of the box.
	 */
	std::string toString() const {
		return "[" + std::to_string(min.x) + ", " + std::to_string(min.y) + ", " + std::to_string(min.z) + " | " +
					 std::to_string(max.x) + ", " + std::to_string(max.y) + ", " + std::to_string(max.z) + "]";
	}

	/**
	 * Returns the box that is the result of interpolating between the two parameters by percent.
	 * @param start The box to start at.
	 * @param finish The box to end at.
	 * @param percent The percent to interpolate between the two boxes.
	 * @return The interpolated box.
	 */
	static Aabb<T> interpolate(const Aabb<T>& start, const Aabb<T>& finish, float percent) {
		return Aabb(vec_t(ExMath::interpolate(start.min.x, finish.min.x, percent),
						  ExMath::interpolate(start.min.y, finish.min.y, percent),
						  ExMath::interpolate(start.min.z, finish.min.z, percent)),
					vec_t(ExMath::interpolate(start.max.x, finish.max.x, percent),
						  ExMath::interpolate(start.max.y, finish.max.y, percent),
						  ExMath::interpolate(start.max.z, finish.max.z, percent)));
	}
};

//For compatibility, don't use in new code.
//Aabb<float> is two characters shorter anyway.
typedef Aabb<float> AxisAlignedBB;

/**
 * Prints an axis-aligned bounding box, in the standard way to print such things.
 * If you're explicitly calling this, you're probably doing something wrong.
 * @param out The output stream to print to.
 * @param box The box to print.
 * @return The passed in output stream.
 */
template<typename T>
std::ostream& operator<<(std::ostream& out, const Aabb<T>& box) {
	out << box.toString();
	return out;
}
