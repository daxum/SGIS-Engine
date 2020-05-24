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

#include "RenderManager.hpp"
#include "Engine.hpp"
#include "Models/ModelManager.hpp"

void RenderManager::onComponentAdd(std::shared_ptr<Component> comp) {
	std::shared_ptr<RenderComponent> renderComp = std::static_pointer_cast<RenderComponent>(comp);

	getComponentSet(renderComp->getModel()).push_back(renderComp.get());
	renderComponentSet.push_back(renderComp.get());
	renderComp->setManager(this);
}

void RenderManager::onComponentRemove(std::shared_ptr<Component> comp) {
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

void RenderManager::reloadComponent(const RenderComponent* renderComp, const Model& oldModel) {
	const Model& model = renderComp->getModel();

	removeComponent(renderComp, oldModel);
	getComponentSet(model).push_back(renderComp);
}

void RenderManager::removeComponent(const RenderComponent* comp, const Model& oldModel) {
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
		const Buffer* buffer = oldModel.mesh->getBufferInfo().vertex;
		const std::string& shader = oldModel.material->shader;

		renderComponents.at(buffer).at(shader).erase(oldModel.material);
	}
}

std::vector<const RenderComponent*>& RenderManager::getComponentSet(const Model& model) {
	const Buffer* buffer = model.mesh->getBufferInfo().vertex;
	const std::string& shader = model.material->shader;

	if (!renderComponents.count(buffer)) {
		//It was inevitable.
		renderComponents.emplace(buffer, std::unordered_map<std::string, std::unordered_map<const Material*, std::vector<const RenderComponent*>>>());
	}

	auto& shaderMap = renderComponents.at(buffer);
	auto& modelMap = shaderMap[shader];

	if (!modelMap.count(model.material)) {
		modelMap.emplace(model.material, std::vector<const RenderComponent*>());
	}

	return modelMap.at(model.material);
}
