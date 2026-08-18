#pragma once

#include <string>

#include "CDrawStyle.h"
#include "CMesh.h"
#include "ecs/MEcs.h"

namespace rig {
namespace obj {

struct IoResult {
	bool ok = false;
	std::string error;
	std::string warning;
};

/**
 * @brief Load a Wavefront OBJ into a `CMesh` (tinyobjloader).
 * @details Triangulates on read, then welds coincident verts and coplanar
 * islands into n-gon `loops` (a box becomes 6 faces). Fills `texcoords` when
 * present. When materials exist, fills `faceColors` from diffuse.
 * Never fills `facePalette` (RigKit-only; not part of Wavefront).
 * @param path File path (.obj). MTL is resolved relative to the OBJ.
 */
IoResult load(const std::string& path, rigkit::ecs::CMesh& outMesh);

/**
 * @brief Write `CMesh` to a simple Wavefront OBJ.
 * @details Prefers authored n-gon `loops`; otherwise triangle indices.
 * Geometry (+ UVs) only — does not write materials, `faceColors`, or `facePalette`.
 * Palette indices are RigKit-only; use `.rig` / document codecs to keep them.
 */
IoResult save(const std::string& path, const rigkit::ecs::CMesh& mesh);

/**
 * @brief Load OBJ and spawn a mesh entity (`CTransform` + `CMesh` + `CDrawStyle`).
 * @return Entity id, or `entt::null` on failure (see `outResult`).
 */
entt::entity makeMeshFromFile(rigkit::MEcs& ecs, const std::string& path,
							  const rigkit::ecs::CDrawStyle& style = {},
							  const std::string& name = {}, IoResult* outResult = nullptr);

} // namespace obj
} // namespace rig
