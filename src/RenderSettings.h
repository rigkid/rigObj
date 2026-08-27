#pragma once

#include <vector>

#include "ecs/PropertyReflection.h"

namespace rigkit {

/**
 * @brief Session knobs for OBJ load present. Registered as `rigObj.render`.
 * @details Import/present choices for this pack - not scene components.
 * `CMesh::faceColors` stay on the entity after load.
 */
struct RenderSettings {
	/// When false, skip MTL diffuse so `CDrawStyle` fill is the albedo.
	bool applyMtlColors = true;

	std::vector<sProp> GetProperties() {
		return {{0, "Apply MTL Colors", EPT_BOOL, &applyMtlColors}};
	}
};

} // namespace rigkit
