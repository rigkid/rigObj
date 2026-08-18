#include "rigObj.h"
#include "core/RigKitEngine.h"
#include "core/pack/PackRegistry.h"
#include <spdlog/spdlog.h>

namespace rigkit {

rigObj::rigObj() : IPack("rigObj") {
	

}

bool rigObj::init() {
	spdlog::info("[rigObj] init");
	return true;
}

void rigObj::setup() {
	spdlog::info("[rigObj] setup (IO helpers; no systems)");
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
