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

#include <memory>
#include <string>
#include <unordered_map>

#include "Components/Component.hpp"
#include "ObjectPhysicsInterface.hpp"

struct ObjectState {
	virtual ~ObjectState() {}

	/**
	 * Called whenever a shader uses a uniform provider type of OBJECT_STATE.
	 * Gets a pointer to the value to be passed into the shader.
	 * @param name The name of the value to retrieve.
	 * @return A pointer to the requested value, nullptr if the value doesn't
	 *     exist.
	 */
	virtual const void* getRenderValue(const std::string& name) const = 0;
};

//An object in a world. Stores rendering, physics, etc.
class Object : public std::enable_shared_from_this<Object> {
public:
	/**
	 * Creates an object.
	 */
	Object();

	/**
	 * Retrieves the component with the requested name, or null if it doesn't exist.
	 * @param name The name of the component.
	 * @return A pointer to the component for this object, will be null if the component isn't found.
	 */
	template <typename T>
	std::shared_ptr<T> getComponent(const std::string& name = T::getName()) {
		static_assert(std::is_base_of<Component, T>::value, "Object::getComponent called with non-component type");

		if (components.count(name)) {
			return std::static_pointer_cast<T>(components.at(name));
		}
		else {
			return std::shared_ptr<T>();
		}
	}

	/**
	 * Constructs a pointer to a component and adds it to the object.
	 * @param args The arguments to one of the constructors of T.
	 */
	template<typename T, class... Args>
	void addComponent(Args&&... args) {
		addComponent(std::make_shared<T>(std::forward<Args>(args)...));
	}

	/**
	 * Adds a component to the object. This should usually be done before the object is
	 * added to the world. The object will set itself as the component's parent.
	 * @param component The component to add.
	 */
	template<typename T>
	void addComponent(std::shared_ptr<T> component) {
		static_assert(std::is_base_of<Component, T>::value, "Attempted to add component which wasn't a Component!");

		if (components.count(T::getName())) {
			throw std::runtime_error("Duplicate component of type " + T::getName() + " added!");
		}

		component->setParent(shared_from_this());
		components.emplace(T::getName(), component);
	}

	/**
	 * Returns the physics interface for this object. Primarily used by RenderComponents.
	 * Should never be null.
	 * @return The object's physics interface.
	 */
	ObjectPhysicsInterface* getPhysics();
	const ObjectPhysicsInterface* getPhysics() const;

	/**
	 * Returns whether the object has a set physics interface.
	 * @return true if the object has a physics interface, false otherwise.
	 */
	bool hasPhysics() const { return physics != nullptr; }

	/**
	 * Sets the physics for the object to be the selected component.
	 * Remove the static pointer cast below to learn why multi inheritance is bad.
	 * @param component The name of the component to set as the physics provider.
	 */
	template<typename T>
	void setPhysics(const std::string& component) {
		static_assert(std::is_base_of<Component, T>::value && std::is_base_of<ObjectPhysicsInterface, T>::value, "Object::setPhysics called with bad type!");
		physics = (ObjectPhysicsInterface*) std::static_pointer_cast<T>(components.at(component)).get();
	}

	/**
	 * Sets the physics interface for the object. This should never really need to be
	 * called outside of the engine except for in rare circumstances.
	 * The set interface will not be deleted by the object.
	 * @param phys The new physics interface for the object.
	 */
	void setPhysics(ObjectPhysicsInterface* phys) { physics = phys; }

	/**
	 * Constructs and sets the state of the object.
	 * @param args The arguments to the state's constructor.
	 */
	template<typename T, class... Args>
	void setState(Args&&... args) {
		setState(std::make_shared<T>(std::forward<Args>(args)...));
	}

	/**
	 * Sets the state for this object.
	 * @param newState The new state variable, user defined. Can be anything.
	 */
	template<typename T>
	void setState(std::shared_ptr<T> newState) {
		static_assert(std::is_base_of<ObjectState, T>::value, "Object::setState called with non-ObjectState type!");
		state = newState;
	}

	/**
	 * Returns the previously set state pointer, or null if none was set.
	 * @return The user-specified state pointer.
	 */
	template<typename T = ObjectState>
	std::shared_ptr<T> getState() {
		static_assert(std::is_base_of<ObjectState, T>::value, "Object::getState called with bad type!");
		return std::static_pointer_cast<T>(state);
	}

	/**
	 * Same as the above, but const.
	 * @return A const version of the user-set state pointer.
	 */
	template<typename T = ObjectState>
	std::shared_ptr<const T> getState() const {
		static_assert(std::is_base_of<ObjectState, T>::value, "Object::getState called with bad type!");
		return std::static_pointer_cast<T>(state);
	}

private:
	//Used by objects with no set physics interface. Completely stateless.
	static ObjectPhysicsInterface defaultInterface;

	//The map of components for this object. Component names should be in Component.hpp.
	std::unordered_map<std::string, std::shared_ptr<Component>> components;

	//The physics interface for the object.
	ObjectPhysicsInterface* physics;

	//User-defined object state.
	std::shared_ptr<ObjectState> state;
};
