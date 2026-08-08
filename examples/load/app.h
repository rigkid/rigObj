#pragma once
#include <entt/entt.hpp>
#include "core/U_core.h"

/**
 * @brief rigObj hero — low-poly landing pad loaded from Wavefront OBJ.
 * @details Rocket, pad, and crates come from `data/*.obj` (+ MTL diffuse →
 * `CMesh::faceColors`). Orbiting camera and a circling banded light show the
 * meshes under rigRender3D after a real file load.
 */
class LoadApp : public rigkit::IApp {
  public:
	LoadApp() {
		window().width = 960;
		window().height = 600;
		window().title = "rigObj — load";
	}
	void setup() override;
	void update(float dt) override;
	void draw() override {}

  private:
	entt::entity m_camera = entt::null;
	entt::entity m_light = entt::null;
	entt::entity m_rocket = entt::null;
	entt::entity m_pad = entt::null;
	entt::entity m_crateA = entt::null;
	entt::entity m_crateB = entt::null;
	float m_time = 0.f;
};
