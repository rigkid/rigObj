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
		settings().graphics.showFps = true;
		settings().graphics.clearColor = {0.04f, 0.045f, 0.05f, 1.0f};
	}
	void setup() override;
	void update(float dt) override;
	void draw() override {}

  protected:
	void registerUIActions(rigkit::ecs::SEvent&) override;

  private:
	void reloadBunny();

	entt::entity m_camera = entt::null;
	entt::entity m_mesh = entt::null;
	rig::OrbitNavState m_nav;
	float m_wheel = 0.f;
	bool m_fDown = false;
	bool m_ctrlRDown = false;
};
