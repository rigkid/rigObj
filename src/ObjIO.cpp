#include "ObjIO.h"

#include "CTransform.h"
#include "rig/create.h"

#include <fstream>

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

} // namespace

IoResult load(const std::string& path, rigkit::ecs::CMesh& outMesh) {
	IoResult result;
	outMesh = {};
	outMesh.mode = rigkit::ecs::CMesh::Mode::Triangles;

	tinyobj::ObjReaderConfig config;
	config.triangulate = true;
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

	bool anyUv = false;
	bool anyColor = false;

	for (const auto& shape : shapes) {
		size_t indexOffset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
			const size_t fv = shape.mesh.num_face_vertices[f];
			if (fv != 3) {
				indexOffset += fv;
				continue;
			}

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

			for (size_t v = 0; v < 3; ++v) {
				const tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
				glm::vec3 pos{0.f};
				if (idx.vertex_index >= 0) {
					const size_t vi = static_cast<size_t>(idx.vertex_index);
					pos = {attrib.vertices[3 * vi + 0], attrib.vertices[3 * vi + 1],
						   attrib.vertices[3 * vi + 2]};
				}
				outMesh.positions.push_back(pos);

				glm::vec2 uv{0.f};
				if (idx.texcoord_index >= 0) {
					const size_t ti = static_cast<size_t>(idx.texcoord_index);
					uv = {attrib.texcoords[2 * ti + 0], attrib.texcoords[2 * ti + 1]};
					anyUv = true;
				}
				outMesh.texcoords.push_back(uv);
			}
			indexOffset += fv;
		}
	}

	if (!anyUv) {
		outMesh.texcoords.clear();
	}
	if (!anyColor) {
		outMesh.faceColors.clear();
	}

	if (outMesh.positions.empty()) {
		result.error = "OBJ contained no triangle faces";
		return result;
	}

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

	auto emitFace = [&](uint32_t i0, uint32_t i1, uint32_t i2) {
		// OBJ indices are 1-based.
		const uint32_t a = i0 + 1;
		const uint32_t b = i1 + 1;
		const uint32_t c = i2 + 1;
		if (hasUv) {
			out << "f " << a << '/' << a << ' ' << b << '/' << b << ' ' << c << '/' << c << '\n';
		} else {
			out << "f " << a << ' ' << b << ' ' << c << '\n';
		}
	};

	if (mesh.indices.empty()) {
		for (size_t i = 0; i + 2 < mesh.positions.size(); i += 3) {
			emitFace(static_cast<uint32_t>(i), static_cast<uint32_t>(i + 1),
					 static_cast<uint32_t>(i + 2));
		}
	} else {
		for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
			emitFace(mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]);
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

	auto e = makeMesh(ecs, std::move(mesh.positions), std::move(mesh.indices), mesh.mode, draw,
					  name);
	auto& stored = ecs.getComponent<rigkit::ecs::CMesh>(e);
	stored.faceColors = std::move(mesh.faceColors);
	stored.texcoords = std::move(mesh.texcoords);
	return e;
}

} // namespace obj
} // namespace rig
