#pragma once

#include "core/pack/IPack.h"
#include "RenderSettings.h"

namespace rigkit {

/**
 * @brief Code pack - Wavefront OBJ load/save for `CMesh` (tinyobjloader).
 * @details No systems. Apps call `rig::obj::load` / `save`. Depends on rigComponent.
 * Session knobs live on `RenderSettings` (`rigObj.render` in Preferences).
 */
class rigObj : public IPack {
  public:
	rigObj();
	bool init() override;
	void setup() override;
	void cleanup() override;

	const RenderSettings& renderSettings() const { return m_render; }
	RenderSettings& renderSettings() { return m_render; }

	/// Live pack while setup has run; `load` reads MTL prefs from here.
	static const RenderSettings* liveRenderSettings();

  private:
	RenderSettings m_render;
};

} // namespace rigkit
