#include "rigObj.h"

#include "core/pack/PackRegistry.h"
#include "core/RigKitEngine.h"
#include "core/util/MSettings.h"

#include <spdlog/spdlog.h>

namespace rigkit {
namespace {
rigObj* g_live = nullptr;
}

rigObj::rigObj() : IPack("rigObj") {
	

}

const RenderSettings* rigObj::liveRenderSettings() {
	return g_live ? &g_live->m_render : nullptr;
}

bool rigObj::init() {
	spdlog::info("[rigObj] init");
	return true;
}

void rigObj::setup() {
	g_live = this;
	if (auto* engine = getEngine()) {
		if (auto* settings = engine->getSettingsManager()) {
			settings->registerPreferences("rigObj.render", "OBJ", &m_render);
		}
	}
	spdlog::info("[rigObj] setup (IO helpers; no systems)");
}

void rigObj::cleanup() {
	if (auto* engine = getEngine()) {
		if (auto* settings = engine->getSettingsManager()) {
			settings->unregisterPreferences("rigObj.render");
		}
	}
	if (g_live == this) {
		g_live = nullptr;
	}
}

} // namespace rigkit

namespace {
struct rigObjRegistrar {
	rigObjRegistrar() {
		rigkit::PackRegistry::instance().addFactory("rigObj", []() {
			return std::shared_ptr<rigkit::IPack>(std::make_shared<rigkit::rigObj>());
		});
	}
};
static rigObjRegistrar rigObj_auto_reg;
} // namespace
