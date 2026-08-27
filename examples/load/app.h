#pragma once
#include <entt/entt.hpp>
#include "core/U_core.h"
#include "packs/rigSystems/src/OrbitNav.h"

/**
 * @brief rigObj hero - Stanford bunny from Wavefront OBJ.
 * @details `data/bunny.obj` (+ MTL diffuse to `CMesh::faceColors`). Orbit camera
 * (mouse orbit / pan / dolly) presents it under rigRender3D after a real file load.
 */
class LoadApp : public rigkit::IApp {
  public:
	LoadApp() {
		window().width = 1280;
		window().height = 720;
		window().title = "rigObj - load";
	}
	void setup() override;
	void update(float dt) override;
	void draw() override {}

  private:
	entt::entity m_camera = entt::null;
	rig::OrbitNavState m_nav;
	float m_wheel = 0.f;
	bool m_fDown = false;
};
