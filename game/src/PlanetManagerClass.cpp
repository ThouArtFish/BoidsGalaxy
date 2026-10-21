#include "../include/PlanetManagerClass.hpp"

PlanetManager::PlanetManager(size_t num_planets, const glm::vec3& arm, const glm::vec3& axis) : num_planets(num_planets) {
	const size_t v = glm::max((size_t)4, planet_vertical_strips), h = glm::max((size_t)4, planet_horizontal_strips);
	const float v_angle = glm::radians((1.0f / (float)v) * 360.0f), h_angle = glm::radians((1.0f / (float)h) * 180.0f);
	const glm::mat3 v_rot = glm::mat3(glm::rotate(glm::mat4(1.0f), v_angle, arm)), h_rot = glm::mat3(glm::rotate(glm::mat4(1.0f), h_angle, axis));

	glm::vec3 vertex = arm;
	std::vector<TAGMesh::Vertex> vertices(2 + v * (h - 1));
	vertices[0] = { .position = vertex, .normal = vertex };
	vertices[1] = { .position = -vertex, .normal = -vertex };
	size_t index = 2;
	for (size_t i = 1; i < h; i++) {
		vertex = h_rot * vertex;
		for (size_t j = 0; j < v; j++) {
			vertex = v_rot * vertex;
			vertices[index++] = { .position = vertex, .normal = vertex };
		}
	}

	std::vector<TAGMesh::Fragment> frags((h - 1) * v * 2);
	GLuint gv = (GLuint)v, gh = (GLuint)h;
	index = 0;
	for (GLuint i = 2; i < gv + 2; i++) {
		frags[index++] = { 0, i, (i == gv + 1 ? 2 : i + 1) };
	}
	for (GLuint i = 1 + gv * (gh - 1); i > 1 + gv * (gh - 2); i--) {
		frags[index++] = { 1, i, (i == 2 + gv * (gh - 2) ? (1 + gv * (gh - 1)) : i - 1) };
	}
	for (GLuint i = 2; i < (gh - 2) * gv + 2; i++) {
		GLuint layer = (i - 2) / gv;
		frags[index++] = { i, i + gv, ((i + gv - 1) / gv != layer + 1 ? i + 1 : i + gv + 1) };
		frags[index++] = { ((i - 1) / gv == layer + 1 ? i - gv + 1 : i + 1), i, ((i + gv - 1) / gv != layer + 1 ? i + 1 : i + gv + 1) };
	}

	planet_model.addMesh(PLANET_MESH_NAME, vertices, frags, { .colour = glm::vec3(0.0f, 1.0f, 0.0f) });
}

void PlanetManager::generatePlanets() {
	std::vector<TAGModel::Object> planets(num_planets);
	for (TAGModel::Object& planet : planets) {
		planet.position = {
			TAGUtil::randomRangeFloat(game_bounds.min.x, game_bounds.max.x),
			TAGUtil::randomRangeFloat(game_bounds.min.y, game_bounds.max.y),
			TAGUtil::randomRangeFloat(game_bounds.min.z, game_bounds.max.z)
		};
		planet.scale = TAGUtil::randomRangeFloat(min_radius, max_radius);
	}
	planet_model.setAllInstances(planets, PLANET_MESH_NAME);
	planet_model.updateInstanceBuffer(PLANET_MESH_NAME);
}

void PlanetManager::drawAll(const TAGShaderManager::Shader& shader) {
	planet_model.drawAll(shader, PLANET_MESH_NAME);
}
