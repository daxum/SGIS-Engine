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

#include <glm/glm.hpp>

#include "Component.hpp"
#include "Model.hpp"

class RenderComponentManager;

class RenderComponent : public Component, std::enable_shared_from_this<RenderComponent> {
public:
	/**
	 * Creates a RenderComponent.
	 * @param model The name of the model to use to render this object.
	 * @param color The color to render the object as.
	 * @param renderScale The scale of the object.
	 */
	RenderComponent(std::string model, glm::vec3 color = glm::vec3(1.0, 1.0, 1.0), glm::vec3 renderScale = glm::vec3(1.0, 1.0, 1.0));

	/**
	 * Similar to the constructor above, but generates a model based on the given text instead.
	 * @param text The text to render. Supports newlines.
	 * @param font The font for the text.
	 * @param shader The shader to render the text with.
	 * @param color The color to render the object as.
	 * @param renderScale The scale of the object.
	 */
	RenderComponent(const std::u32string& text, const std::string& font, const std::string& shader, glm::vec3 color = glm::vec3(1.0, 1.0, 1.0), glm::vec3 renderScale = glm::vec3(1.0, 1.0, 1.0));

	/**
	 * Destructor.
	 */
	~RenderComponent();

	/**
	 * Returns the translation of this object.
	 * @return A translation vector for this object.
	 */
	glm::vec3 getTranslation();

	/**
	 * Returns the rotation of this object.
	 * @return A quaternion for this object's rotation.
	 */
	glm::quat getRotation();

	/**
	 * Returns the scale of this object.
	 * @return The scale of this object.
	 */
	glm::vec3 getScale();

	/**
	 * Returns the color of this object.
	 * @return The color of this object.
	 */
	glm::vec3 getColor();

	/**
	 * Returns the model to be used in rendering this object.
	 * @return The model.
	 */
	const Model& getModel() { return model; }

	/**
	 * Changes the component's model to the specified one.
	 * @param newModel The new model to use.
	 */
	void setModel(const Model& newModel);

	/**
	 * Sets the model to the given text.
	 * @param text The text to set the model to.
	 * @param font The font to use.
	 * @param shader The shader to use.
	 */
	void setTextModel(const std::u32string& text, const std::string& font, const std::string& shader);

	/**
	 * Same as above, reuses previous font and shader.
	 */
	void setTextModel(const std::u32string& text);

	/**
	 * Only to be called from RenderComponentManager.
	 */
	void setManager(RenderComponentManager* renderManager) { manager = renderManager; }

private:
	//Which model to use for this object.
	Model model;

	//Whether the model was generated dynamically and needs to be freed.
	bool textModel;

	//The color of the object
	glm::vec3 color;

	//The scale of the object's model.
	glm::vec3 scale;

	//The manager for this component, null if none.
	RenderComponentManager* manager;
};
