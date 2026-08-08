#pragma once

#include "core/pack/IPack.h"

namespace rigkit {

/**
 * @brief Code pack — Wavefront OBJ load/save for `CMesh` (tinyobjloader).
 * @details No systems. Apps call `rig::obj::load` / `save`. Depends on rigComponent.
 */
class rigObj : public IPack {
  public:
	rigObj();
	bool init() override;
	void setup() override;
};

} // namespace rigkit
