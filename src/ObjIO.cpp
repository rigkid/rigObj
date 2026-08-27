#include "ObjIO.h"

#include "CTransform.h"
#include "MeshFaces.h"
#include "rig/create.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace rig {
namespace obj {
namespace {

glm::vec4 diffuseColor(const tinyobj::material_t* mat) {
	if (!mat) {
		return {1.f, 1.f, 1.f, 1.f};
	}
	return {mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], 1.f};
}

bool sameUv(const glm::vec2& a, const glm::vec2& b) {
	return std::fabs(a.x - b.x) < 1e-6f && std::fabs(a.y - b.y) < 1e-6f;
}

} // namespace

IoResult load(const std::string& path, rigkit::ecs::CMesh& outMesh) {
	IoResult result;
	outMesh = {};
	outMesh.mode = rigkit::ecs::CMesh::Mode::Triangles;

	tinyobj::ObjReaderConfig config;
	config.triangulate = false;
	config.vertex_color = false;

	tinyobj::ObjReader reader;
	if (!reader.ParseFromFile(path, config)) {
		result.error = reader.Error().empty() ? "failed to parse OBJ" : reader.Error();
		return result;
	}
	result.warning = reader.Warning();

	const auto& attrib = reader.GetAttrib();
	const auto& shapes = reader.GetShapes();
	const auto& materials = reader.GetMaterials();

	const size_t nVert = attrib.vertices.size() / 3;
	if (nVert == 0) {
		result.error = "OBJ contained no vertices";
		return result;
	}
	outMesh.positions.resize(nVert);
	for (size_t i = 0; i < nVert; ++i) {
		outMesh.positions[i] = {attrib.vertices[3 * i + 0], attrib.vertices[3 * i + 1],
								attrib.vertices[3 * i + 2]};
	}

	const size_t nUv = attrib.texcoords.size() / 2;
	std::vector<uint8_t> uvSet;
	bool anyUv = false;
	bool anyColor = false;

	size_t nFaces = 0;
	size_t nCorners = 0;
	for (const auto& shape : shapes) {
		nFaces += shape.mesh.num_face_vertices.size();
		nCorners += shape.mesh.indices.size();
	}
	outMesh.loops.reserve(nCorners);
	outMesh.loopSizes.reserve(nFaces);
	outMesh.faceColors.reserve(nFaces);

	auto corner = [&](const tinyobj::index_t& idx) -> uint32_t {
		const uint32_t vi = static_cast<uint32_t>(idx.vertex_index);
		if (idx.texcoord_index < 0 || static_cast<size_t>(idx.texcoord_index) >= nUv) {
			return vi;
		}
		const size_t ti = static_cast<size_t>(idx.texcoord_index);
		const glm::vec2 uv{attrib.texcoords[2 * ti + 0], attrib.texcoords[2 * ti + 1]};
		anyUv = true;
		if (outMesh.texcoords.size() != outMesh.positions.size()) {
			outMesh.texcoords.assign(outMesh.positions.size(), {0.f, 0.f});
			uvSet.assign(outMesh.positions.size(), 0);
		}
		if (!uvSet[vi]) {
			outMesh.texcoords[vi] = uv;
			uvSet[vi] = 1;
			return vi;
		}
		if (sameUv(outMesh.texcoords[vi], uv)) {
			return vi;
		}
		const uint32_t ni = static_cast<uint32_t>(outMesh.positions.size());
		outMesh.positions.push_back(outMesh.positions[vi]);
		outMesh.texcoords.push_back(uv);
		uvSet.push_back(1);
		return ni;
	};

	for (const auto& shape : shapes) {
		size_t indexOffset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
			const size_t fv = shape.mesh.num_face_vertices[f];
			if (fv < 3 || indexOffset + fv > shape.mesh.indices.size()) {
				indexOffset += fv;
				continue;
			}

			bool ok = true;
			for (size_t v = 0; v < fv; ++v) {
				const tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
				if (idx.vertex_index < 0 || static_cast<size_t>(idx.vertex_index) >= nVert) {
					ok = false;
					break;
				}
			}
			if (!ok) {
				indexOffset += fv;
				continue;
			}
			for (size_t v = 0; v < fv; ++v) {
				outMesh.loops.push_back(corner(shape.mesh.indices[indexOffset + v]));
			}
			indexOffset += fv;
			outMesh.loopSizes.push_back(static_cast<uint32_t>(fv));

			const int matId = f < shape.mesh.material_ids.size() ? shape.mesh.material_ids[f] : -1;
			const tinyobj::material_t* mat =
				(matId >= 0 && matId < static_cast<int>(materials.size())) ? &materials[matId]
																		   : nullptr;
			if (mat) {
				anyColor = true;
				outMesh.faceColors.push_back(diffuseColor(mat));
			} else {
				outMesh.faceColors.push_back({1.f, 1.f, 1.f, 1.f});
			}
		}
	}

	if (!anyUv) {
		outMesh.texcoords.clear();
	}
	if (!anyColor) {
		outMesh.faceColors.clear();
	}

	if (outMesh.loopSizes.empty()) {
		result.error = "OBJ contained no faces";
		return result;
	}

	rigkit::ecs::meshTriangulate(outMesh);

	result.ok = true;
	return result;
}

IoResult save(const std::string& path, const rigkit::ecs::CMesh& mesh) {
	IoResult result;
	if (mesh.mode != rigkit::ecs::CMesh::Mode::Triangles) {
		result.error = "save OBJ supports triangle meshes only";
		return result;
	}
	if (mesh.positions.empty()) {
		result.error = "empty mesh";
		return result;
	}

	std::ofstream out(path, std::ios::out | std::ios::trunc);
	if (!out) {
		result.error = "failed to open file for write";
		return result;
	}

	out << "# RigKit rigObj\n";
	for (const auto& p : mesh.positions) {
		out << "v " << p.x << ' ' << p.y << ' ' << p.z << '\n';
	}

	const bool hasUv = mesh.texcoords.size() == mesh.positions.size();
	if (hasUv) {
		for (const auto& t : mesh.texcoords) {
			out << "vt " << t.x << ' ' << t.y << '\n';
		}
	}

	auto emitFace = [&](const uint32_t* idx, uint32_t n) {
		out << 'f';
		for (uint32_t i = 0; i < n; ++i) {
			const uint32_t a = idx[i] + 1;
			if (hasUv) {
				out << ' ' << a << '/' << a;
			} else {
				out << ' ' << a;
			}
		}
		out << '\n';
	};

	if (!mesh.loopSizes.empty()) {
		uint32_t off = 0;
		for (uint32_t n : mesh.loopSizes) {
			if (n >= 3 && off + n <= mesh.loops.size()) {
				emitFace(mesh.loops.data() + off, n);
			}
			off += n;
		}
	} else if (mesh.indices.empty()) {
		for (size_t i = 0; i + 2 < mesh.positions.size(); i += 3) {
			const uint32_t t[3] = {static_cast<uint32_t>(i), static_cast<uint32_t>(i + 1),
								   static_cast<uint32_t>(i + 2)};
			emitFace(t, 3);
		}
	} else {
		for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
			const uint32_t t[3] = {mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]};
			emitFace(t, 3);
		}
	}

	result.ok = true;
	return result;
}

entt::entity makeMeshFromFile(rigkit::MEcs& ecs, const std::string& path,
							  const rigkit::ecs::CDrawStyle& style, const std::string& name,
							  IoResult* outResult) {
	rigkit::ecs::CMesh mesh;
	IoResult loaded = load(path, mesh);
	if (outResult) {
		*outResult = loaded;
	}
	if (!loaded.ok) {
		return entt::null;
	}

	rigkit::ecs::CDrawStyle draw = style;
	if (!draw.hasFill && !draw.hasStroke) {
		draw = fill(1.f, 1.f, 1.f);
	}

	const auto e = ecs.createEntity(name);
	ecs.addComponent<rigkit::ecs::CTransform>(e);
	ecs.addComponent<rigkit::ecs::CMesh>(e, std::move(mesh));
	ecs.addComponent<rigkit::ecs::CDrawStyle>(e, draw);
	return e;
}

} // namespace obj
} // namespace rig
