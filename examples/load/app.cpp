#include "app.h"

#include <cmath>
#include <filesystem>
#include <string>

#include <spdlog/spdlog.h>
#include "core/RigKitEngine.h"
#include "core/pack/MPack.h"
#include "core/util/AppPaths.h"
#include "packs/rigComponent/src/CLight.h"
#include "packs/rigComponent/src/COrbitDrive.h"
#include "packs/rigComponent/src/CTransform.h"
#include "packs/rigComponent/src/rig.h"
#include "packs/rigComponent/src/rigComponent.h"
#include "packs/rigObj/src/ObjIO.h"
#include "packs/rigObj/src/rigObj.h"
#include "packs/rigRender3D/src/rigRender3D.h"
#include "packs/rigSystems/src/rigSystems.h"

namespace {

entt::entity loadObj(rigkit::MEcs& ecs, const std::filesystem::path& path,
					 const std::string& name) {
	rig::obj::IoResult io;
	auto e = rig::obj::makeMeshFromFile(ecs, path.string(), {}, name, &io);
	if (!io.ok) {
		spdlog::error("[load] {}: {}", path.filename().string(), io.error);
	} else if (!io.warning.empty()) {
		spdlog::warn("[load] {}: {}", path.filename().string(), io.warning);
	} else {
		spdlog::info("[load] loaded {}", path.filename().string());
	}
	return e;
}

} // namespace

void LoadApp::setup() {
	spdlog::info("load — rigObj landing pad (OBJ + MTL → CMesh faceColors)");
	m_engine->setClearColor(0.04f, 0.05f, 0.09f, 1.0f);

	auto* packs = m_engine->getPackManager();
	if (!packs) {
		return;
	}
	packs->registerPack(std::make_shared<rigkit::rigComponent>());
	packs->registerPack(std::make_shared<rigkit::rigSystems>());
	packs->registerPack(std::make_shared<rigkit::rigRender3D>());
	packs->registerPack(std::make_shared<rigkit::rigObj>());
	packs->initAll();
	packs->setupAll();

	auto* ecs = m_engine->getECSManager();
	if (!ecs) {
		return;
	}

	m_camera = rig::makeCamera(*ecs, {0.0f, 2.4f, 5.5f}, true, "load-camera");
	rigkit::ecs::COrbitDrive orbit;
	orbit.speed = 0.22f;
	orbit.radius = 5.5f;
	orbit.height = 2.4f;
	ecs->addComponent<rigkit::ecs::COrbitDrive>(m_camera, orbit);
	rig::lookAt(ecs->getComponent<rigkit::ecs::CTransform>(m_camera),
				{0.0f, orbit.height, orbit.radius}, {0.0f, 0.5f, 0.0f});

	m_light =
		rig::makeLight(*ecs, {3.0f, 3.5f, 2.0f}, rigkit::ecs::CLight::Type::Point, "load-light");
	auto& lightData = ecs->getComponent<rigkit::ecs::CLight>(m_light);
	lightData.intensity = 1.35f;
	lightData.ambient = 0.28f;
	lightData.banded = true;
	lightData.bands = 4;
	lightData.colorR = 1.0f;
	lightData.colorG = 0.96f;
	lightData.colorB = 0.88f;

	const auto data = std::filesystem::path(AppPaths::getDataDir());
	m_pad = loadObj(*ecs, data / "pad.obj", "pad");
	m_rocket = loadObj(*ecs, data / "rocket.obj", "rocket");
	m_crateA = loadObj(*ecs, data / "crate.obj", "crate-a");
	m_crateB = loadObj(*ecs, data / "crate.obj", "crate-b");

	if (m_crateA != entt::null) {
		auto& t = ecs->getComponent<rigkit::ecs::CTransform>(m_crateA);
		t.position = {1.55f, 0.0f, 0.85f};
		t.setEulerRadians({0.0f, 0.4f, 0.0f});
	}
	if (m_crateB != entt::null) {
		auto& t = ecs->getComponent<rigkit::ecs::CTransform>(m_crateB);
		t.position = {1.85f, 0.0f, -0.55f};
		t.setEulerRadians({0.0f, -0.7f, 0.0f});
		t.scale = {0.85f, 0.85f, 0.85f};
	}
}

void LoadApp::update(float dt) {
	m_time += dt;
	auto* ecs = m_engine->getECSManager();
	if (!ecs) {
		return;
	}

	if (m_camera != entt::null && ecs->hasComponent<rigkit::ecs::COrbitDrive>(m_camera)) {
		const auto& orbit = ecs->getComponent<rigkit::ecs::COrbitDrive>(m_camera);
		const float yaw = m_time * orbit.speed;
		const glm::vec3 eye{std::sin(yaw) * orbit.radius, orbit.height,
							std::cos(yaw) * orbit.radius};
		rig::lookAt(ecs->getComponent<rigkit::ecs::CTransform>(m_camera), eye, {0.0f, 0.5f, 0.0f});
	}

	if (m_light != entt::null && ecs->hasComponent<rigkit::ecs::CTransform>(m_light)) {
		auto& lx = ecs->getComponent<rigkit::ecs::CTransform>(m_light);
		const float t = m_time * 0.45f;
		lx.position = {std::cos(t) * 3.2f, 2.8f + 0.5f * std::sin(t * 1.2f), std::sin(t) * 3.2f};
	}

	if (m_rocket != entt::null && ecs->hasComponent<rigkit::ecs::CTransform>(m_rocket)) {
		auto& t = ecs->getComponent<rigkit::ecs::CTransform>(m_rocket);
		// Soft idle: slight yaw drift + a tiny vertical bob on the pad.
		t.position.y = 0.02f + 0.03f * std::sin(m_time * 1.6f);
		t.setEulerRadians({0.0f, m_time * 0.15f, 0.0f});
	}
}
