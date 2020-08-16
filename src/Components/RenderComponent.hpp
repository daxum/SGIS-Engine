/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018, 2019
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
#include "Models/Material.hpp"
#include "Display/Object.hpp"
#include "Models/ModelManager.hpp"

class RenderManager;

class RenderComponent : public Component {
public:
	/**
	 * Required from component.
	 * @return The component's name.
	 */
	static const std::string getName() { return RENDER_COMPONENT_NAME; }

	/**
	 * Creates a RenderComponent.
	 * @param material The name of the material to use to render this object.
	 * @param mesh The name of the object's mesh.
	 * @param renderScale The scale of the object.
	 */
	RenderComponent(const std::string& material, const std::string& mesh, glm::vec3 renderScale = glm::vec3(1.0, 1.0, 1.0));

	/**
	 * Creates a RenderComponent. Same as above, but takes the actual model.
	 * @param model The model to use to render this object.
	 * @param renderScale The scale of the object.
	 */
	RenderComponent(Model model, glm::vec3 renderScale = glm::vec3(1.0, 1.0, 1.0));

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
	glm::mat4 getTransform() const {
		glm::mat4 rotation = glm::mat4_cast(getRotation());
		glm::mat4 translation = glm::translate(glm::mat4(1.0), getTranslation());
		glm::mat4 scale = glm::scale(glm::mat4(1.0), getScale());

		return translation * rotation * scale;
	}

	/**
	 * Sets the renderComponent's scale.
	 * @param newScale The new scale.
	 */
	void setScale(glm::vec3 newScale) { scale = newScale; }

	/**
	 * Returns the model to be used in rendering this object.
	 * @return The model.
	 */
	const Model& getModel() const { return model; }

	/**
	 * Changes the component's model to the specified one.
	 * @param newModel The new model to use.
	 */
	void setModel(Model newModel);

	/**
	 * Only to be called from RenderComponentManager.
	 */
	void setManager(RenderManager* renderManager) { manager = renderManager; }

	/**
	 * Gets the object state, for retrieving uniform values.
	 * @return A pointer to the user-set object state.
	 */
	std::shared_ptr<const ObjectState> getParentState() const { return lockParent()->getState(); }

	/**
	 * Sets the cached visibility value for the component.
	 * Internal only! For disabling rendering of objects,
	 * see setHidden.
	 * @param vis Whether the component is visible.
	 */
	void setVisible(bool vis) const { visible = vis; }

	/**
	 * Gets whether the component was marked as visible, to determine
	 * whether it should be rendered.
	 * @return Whether the component can be seen by the camera.
	 */
	bool isVisible() const { return visible; }

	/**
	 * Sets whether the RenderComponent should currently be rendered.
	 * @param newHidden The new hidden state.
	 */
	void setHidden(bool newHidden) { hidden = newHidden; }

	/**
	 * Gets whether the render component has been prevented from rendering.
	 * @return The current hidden state of the component.
	 */
	bool isHidden() const { return hidden; }

private:
	//Which model to use for this object.
	Model model;
	//The scale of the object's model.
	glm::vec3 scale;
	//Whether the RenderComponent was visible in the last frame, cached for the rendering engine.
	mutable bool visible;
	//Whether the RenderComponent should be rendered, for external use.
	bool hidden;
	//The manager for this component, null if none.
	RenderManager* manager;
};
