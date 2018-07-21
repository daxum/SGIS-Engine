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

#include "RenderComponentManager.hpp"
#include "Engine.hpp"
#include "Model.hpp"

void RenderComponentManager::onComponentAdd(std::shared_ptr<Component> comp) {
	std::shared_ptr<RenderComponent> renderComp = std::static_pointer_cast<RenderComponent>(comp);

	getComponentSet(renderComp->getModel()).insert(renderComp);
	renderComponentSet.insert(renderComp);
	renderComp->setManager(this);
}

void RenderComponentManager::onComponentRemove(std::shared_ptr<Component> comp) {
	std::shared_ptr<RenderComponent> renderComp = std::static_pointer_cast<RenderComponent>(comp);

	getComponentSet(renderComp->getModel()).erase(renderComp);
	renderComponentSet.erase(renderComp);
	renderComp->setManager(nullptr);
}

void RenderComponentManager::reloadComponent(std::shared_ptr<RenderComponent> renderComp, std::shared_ptr<const ModelRef> oldModel) {
	std::shared_ptr<const ModelRef> model = renderComp->getModel();

	getComponentSet(oldModel).erase(renderComp);
	getComponentSet(model).insert(renderComp);
}

std::unordered_set<std::shared_ptr<RenderComponent>>& RenderComponentManager::getComponentSet(std::shared_ptr<const ModelRef> model) {
	const std::string& buffer = model->getMesh().getBuffer();
	const std::string& shader = model->getModel().shader;
	const Model* modelPtr = &model->getModel();

	auto& shaderMap = renderComponents[buffer];
	auto& modelMap = shaderMap[shader];

	if (!modelMap.count(modelPtr)) {
		modelMap.insert({modelPtr, std::unordered_set<std::shared_ptr<RenderComponent>>()});
	}

	return modelMap.at(modelPtr);
}
