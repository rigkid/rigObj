#pragma once

#include <string>

#include "ecs/MEcs.h"
#include "CDrawStyle.h"
#include "CMesh.h"

namespace rig {
namespace obj {

struct IoResult {
	bool ok = false;
	std::string error;
	std::string warning;
};

/**
 * @brief Load a Wavefront OBJ into a `CMesh` (tinyobjloader).
 * @details Keeps the file's vertices and faces. Authored n-gons go in `loops`
 * (a box stays 6 faces). Tessellates those loops to triangle `indices` for
 * present. Does not weld coincident verts or merge coplanar islands. Fills
 * `texcoords` when present (splits a vertex only when one OBJ vertex is used
 * with two UVs). When materials exist, fills `faceColors` from diffuse.
 * Never fills `facePalette` (RigKit-only; not part of Wavefront).
 * @param path File path (.obj). MTL is resolved relative to the OBJ.
 */
IoResult load(const std::string& path, rigkit::ecs::CMesh& outMesh);

/**
 * @brief Write `CMesh` to a simple Wavefront OBJ.
 * @details Prefers authored n-gon `loops`; otherwise triangle indices.
 * Geometry (+ UVs) only - does not write materials, `faceColors`, or `facePalette`.
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
