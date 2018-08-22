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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Component.hpp"
#include "Model.hpp"
#include "Object.hpp"

class RenderComponentManager;

class RenderComponent : public Component {
public:
	/**
	 * Creates a RenderComponent.
	 * @param model The name of the model to use to render this object.
	 * @param renderScale The scale of the object.
	 */
	RenderComponent(std::string model, glm::vec3 renderScale = glm::vec3(1.0, 1.0, 1.0));

	/**
	 * Creates a RenderComponent. Same as above, but takes the actual model.
	 * @param model The model to use to render this object.
	 * @param renderScale The scale of the object.
	 */
	RenderComponent(std::shared_ptr<ModelRef> model, glm::vec3 renderScale = glm::vec3(1.0, 1.0, 1.0));

	/**
	 * Returns the translation of this object.
	 * @return A translation vector for this object.
	 */
	glm::vec3 getTranslation() const { return lockParent()->getPhysics()->getTranslation(); }

	/**
	 * Returns the rotation of this object.
	 * @return A quaternion for this object's rotation.
	 */
	glm::quat getRotation() const { return  lockParent()->getPhysics()->getRotation(); }

	/**
	 * Returns the scale of this object.
	 * @return The scale of this object.
	 */
	glm::vec3 getScale() const { return scale; }

	/**
	 * Calculates and returns the object's transform.
	 * @return The tranform to apply to this object.
	 */
	glm::mat4 getTransform() const { return glm::scale(glm::translate(glm::mat4(1.0), getTranslation()) * glm::mat4_cast(getRotation()), getScale()); }

	/**
	 * Sets the renderComponent's scale.
	 * @param newScale The new scale.
	 */
	void setScale(glm::vec3 newScale) { scale = newScale; }

	/**
	 * Returns the model to be used in rendering this object.
	 * @return The model.
	 */
	std::shared_ptr<const ModelRef> getModel() const { return model; }

	/**
	 * Changes the component's model to the specified one.
	 * @param newModel The new model to use.
	 */
	void setModel(std::shared_ptr<ModelRef> newModel);

	/**
	 * Only to be called from RenderComponentManager.
	 */
	void setManager(RenderComponentManager* renderManager) { manager = renderManager; }

	/**
	 * Gets the object state, for retrieving uniform values.
	 * @return A pointer to the user-set object state.
	 */
	std::shared_ptr<const ObjectState> getParentState() const { return lockParent()->getState(); }

private:
	//Which model to use for this object.
	std::shared_ptr<ModelRef> model;
	//The scale of the object's model.
	glm::vec3 scale;
	//The manager for this component, null if none.
	RenderComponentManager* manager;
};
