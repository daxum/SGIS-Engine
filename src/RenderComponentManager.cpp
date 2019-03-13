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

#include <algorithm>

#include "RenderComponentManager.hpp"
#include "Engine.hpp"
#include "Model.hpp"

void RenderComponentManager::onComponentAdd(std::shared_ptr<Component> comp) {
	std::shared_ptr<RenderComponent> renderComp = std::static_pointer_cast<RenderComponent>(comp);

	getComponentSet(renderComp->getModel()).push_back(renderComp.get());
	renderComponentSet.push_back(renderComp.get());
	renderComp->setManager(this);
}

void RenderComponentManager::onComponentRemove(std::shared_ptr<Component> comp) {
	std::shared_ptr<RenderComponent> renderComp = std::static_pointer_cast<RenderComponent>(comp);

	removeComponent(renderComp.get(), renderComp->getModel());
	auto compLoc = std::find(renderComponentSet.begin(), renderComponentSet.end(), comp.get());

	if (compLoc != renderComponentSet.end()) {
		*compLoc = renderComponentSet.back();
		renderComponentSet.pop_back();
	}
	else {
		throw std::runtime_error("Attempt to remove non-present render component");
	}

	renderComp->setManager(nullptr);
}

void RenderComponentManager::reloadComponent(const RenderComponent* renderComp, std::shared_ptr<const ModelRef> oldModel) {
	std::shared_ptr<const ModelRef> model = renderComp->getModel();

	removeComponent(renderComp, oldModel);
	getComponentSet(model).push_back(renderComp);
}

void RenderComponentManager::removeComponent(const RenderComponent* comp, std::shared_ptr<const ModelRef> oldModel) {
	std::vector<const RenderComponent*>& compSet = getComponentSet(oldModel);

	auto compLoc = std::find(compSet.begin(), compSet.end(), comp);

	if (compLoc != compSet.end()) {
		*compLoc = compSet.back();
		compSet.pop_back();
	}
	else {
		throw std::runtime_error("Attempt to remove non-present render component");
	}

	if (compSet.empty()) {
		const std::string& buffer = oldModel->getMesh().getBuffer();
		const std::string& shader = oldModel->getModel().shader;

		renderComponents.at(buffer).at(shader).erase(&oldModel->getModel());
	}
}

std::vector<const RenderComponent*>& RenderComponentManager::getComponentSet(std::shared_ptr<const ModelRef> model) {
	const std::string& buffer = model->getMesh().getBuffer();
	const std::string& shader = model->getModel().shader;
	const Model* modelPtr = &model->getModel();

	auto& shaderMap = renderComponents[buffer];
	auto& modelMap = shaderMap[shader];

	if (!modelMap.count(modelPtr)) {
		modelMap.insert({modelPtr, std::vector<const RenderComponent*>()});
	}

	return modelMap.at(modelPtr);
}
