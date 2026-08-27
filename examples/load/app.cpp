#include "app.h"

#include <filesystem>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "core/IMui.h"
#include "core/RigKitEngine.h"
#include "core/pack/MPack.h"
#include "core/util/AppPaths.h"
#include "packs/rigComponent/src/CCamera.h"
#include "packs/rigComponent/src/CLight.h"
#include "packs/rigComponent/src/CMesh.h"
#include "packs/rigComponent/src/COrbitDrive.h"
#include "packs/rigComponent/src/CTransform.h"
#include "packs/rigComponent/src/rig.h"
#include "packs/rigComponent/src/rigComponent.h"
#include "packs/rigObj/src/ObjIO.h"
#include "packs/rigObj/src/rigObj.h"
#include "packs/rigRender3D/src/rigRender3D.h"
#include "packs/rigSystems/src/OrbitNav.h"
#include "packs/rigSystems/src/rigSystems.h"

namespace {

float* g_wheel = nullptr;

void onScroll(GLFWwindow*, double, double yoff) {
	if (g_wheel) {
		*g_wheel += static_cast<float>(yoff);
	}
}

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

void sit(rigkit::MEcs& ecs, entt::entity e, float height) {
	if (e == entt::null || !ecs.hasComponent<rigkit::ecs::CMesh>(e) ||
		!ecs.hasComponent<rigkit::ecs::CTransform>(e)) {
		return;
	}
	const auto& mesh = ecs.getComponent<rigkit::ecs::CMesh>(e);
	if (mesh.positions.empty()) {
		return;
	}
	glm::vec3 mn = mesh.positions.front();
	glm::vec3 mx = mn;
	for (const auto& p : mesh.positions) {
		mn = glm::min(mn, p);
		mx = glm::max(mx, p);
	}
	const float h = mx.y - mn.y;
	if (h < 1e-6f) {
		return;
	}
	const float s = height / h;
	const glm::vec3 mid = 0.5f * (mn + mx);
	auto& xf = ecs.getComponent<rigkit::ecs::CTransform>(e);
	xf.scale = {s, s, s};
	xf.position = {-mid.x * s, -mn.y * s, -mid.z * s};
}

} // namespace

void LoadApp::setup() {
	spdlog::info("load - Stanford bunny (OBJ + MTL to CMesh)");
	m_engine->setClearColor(settings().graphics.clearColor.r, settings().graphics.clearColor.g,
							settings().graphics.clearColor.b, settings().graphics.clearColor.a);

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

	m_camera = rig::makeOrbitCamera(*ecs, {0.0f, 1.2f, 0.0f}, 5.5f, 0.32f, true, "load-camera");
	auto& orbit = ecs->getComponent<rigkit::ecs::COrbitDrive>(m_camera);
	orbit.yaw = 0.55f;
	orbit.enabled = false;
	auto& cam = ecs->getComponent<rigkit::ecs::CCamera>(m_camera);
	cam.fovYDegrees = 36.0f;

	const auto light =
		rig::makeLight(*ecs, {0.f, 0.f, 0.f}, rigkit::ecs::CLight::Type::Directional, "load-light");
	rig::parentTo(*ecs, light, m_camera);
	auto& lightData = ecs->getComponent<rigkit::ecs::CLight>(light);
	lightData.intensity = 1.15f;
	lightData.ambient = 0.38f;
	lightData.banded = false;
	lightData.colorR = 1.0f;
	lightData.colorG = 0.96f;
	lightData.colorB = 0.90f;

	const auto data = std::filesystem::path(AppPaths::getDataDir());
	m_mesh = loadObj(*ecs, data / "bunny.obj", "bunny");
	sit(*ecs, m_mesh, 2.4f);

	rig::orbitFrameMeshes(*ecs, m_camera);
	auto& framed = ecs->getComponent<rigkit::ecs::COrbitDrive>(m_camera);
	framed.yaw = 0.55f;
	framed.pitch = 0.32f;
	framed.enabled = false;
	rig::orbitApply(*ecs, m_camera);

	if (GLFWwindow* win = m_engine->getWindow()) {
		g_wheel = &m_wheel;
		glfwSetScrollCallback(win, onScroll);
	}
	spdlog::info("load - LMB orbit, Shift+LMB / MMB pan, wheel or RMB dolly, F frame, Ctrl+R reload");
}

void LoadApp::registerUIActions(rigkit::ecs::SEvent&) {
	if (auto* ui = m_engine ? m_engine->getUiManager() : nullptr) {
		ui->registerViewAction("Reload", [this] { reloadBunny(); }, "Ctrl+R");
		ui->registerShortcut("view.reload", "Reload", "Ctrl+R", [this] { reloadBunny(); });
	}
}

void LoadApp::reloadBunny() {
	auto* ecs = m_engine ? m_engine->getECSManager() : nullptr;
	if (!ecs) {
		return;
	}
	const auto path = std::filesystem::path(AppPaths::getDataDir()) / "bunny.obj";
	if (m_mesh == entt::null || !ecs->hasComponent<rigkit::ecs::CMesh>(m_mesh)) {
		m_mesh = loadObj(*ecs, path, "bunny");
		sit(*ecs, m_mesh, 2.4f);
		return;
	}
	rigkit::ecs::CMesh next;
	const auto io = rig::obj::load(path.string(), next);
	if (!io.ok) {
		spdlog::error("[load] reload {}: {}", path.filename().string(), io.error);
		return;
	}
	if (!io.warning.empty()) {
		spdlog::warn("[load] reload {}: {}", path.filename().string(), io.warning);
	}
	auto& mesh = ecs->getComponent<rigkit::ecs::CMesh>(m_mesh);
	const uint32_t revision = mesh.revision + 1;
	mesh = std::move(next);
	mesh.revision = revision;
	sit(*ecs, m_mesh, 2.4f);
	spdlog::info("[load] reloaded {}", path.filename().string());
}

void LoadApp::update(float) {
	auto* ecs = m_engine->getECSManager();
	GLFWwindow* win = m_engine->getWindow();
	if (!ecs || !win || m_camera == entt::null) {
		return;
	}

	double mx = 0.0, my = 0.0;
	glfwGetCursorPos(win, &mx, &my);
	const int lmb = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	const int rmb = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	const int mmb = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
	const int alt = glfwGetKey(win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
					glfwGetKey(win, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
	const int shift = glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
					  glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	const bool f = glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS;

	int winW = 0, winH = 0;
	glfwGetWindowSize(win, &winW, &winH);

	rig::OrbitNavFrame frame;
	frame.mouseX = static_cast<float>(mx);
	frame.mouseY = static_cast<float>(my);
	frame.viewW = static_cast<float>(winW);
	frame.viewH = static_cast<float>(winH);
	frame.orbit = (lmb && !shift) || (alt && lmb);
	frame.pan = (lmb && shift) || mmb || (alt && mmb);
	frame.dolly = rmb || (alt && rmb);
	frame.wheel = m_wheel;
	m_wheel = 0.f;
	rig::orbitNavigate(*ecs, m_camera, frame, m_nav);

	if (f && !m_fDown) {
		rig::orbitFrameMeshes(*ecs, m_camera);
	}
	m_fDown = f;

	const int ctrl = glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
					 glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	const bool ctrlR = ctrl && glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS;
	if (ctrlR && !m_ctrlRDown) {
		reloadBunny();
	}
	m_ctrlRDown = ctrlR;
}
