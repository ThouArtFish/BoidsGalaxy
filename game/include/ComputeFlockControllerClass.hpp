#pragma once

#include <ResourceManagerClass.hpp>
#include <ShaderManagerClass.hpp>
#include <ModelClass.hpp>
#include <MeshClass.hpp>
#include <UtilClass.hpp>
#include "PingPongBufferClass.hpp"
#include "FlockControllerBase.hpp"
#include "QueryManagerClass.hpp"

class ComputeFlockController : public FlockControllerBase {
public:
	struct BoidConstants {
		GLfloat separation_factor, alignment_factor, cohesion_factor;
		GLfloat protected_range_sqr, visible_range_sqr;
		GLfloat border_factor;
		GLfloat min_speed, max_speed;
		alignas(16) glm::vec3 max_margin;
		GLuint total_boids;
		GLfloat dt;

		BoidConstants(const ComputeFlockController* self, float dt);
	};
	struct PlanetConstants {
		GLfloat orbit_pull, orbit_pull_range, tangent_boost;
		GLuint total_planets;

		PlanetConstants(const PlanetManager* planets);
	};
	struct BoidObject {
		glm::vec3 direction;
		GLfloat speed;
	};
	inline static const std::string COMPUTE_BOID_UPDATE_NAME = "computer_boid_update";
	constexpr static float WORK_GROUP_SIZE = 128.0f;

	ComputeFlockController(size_t boid_count);
	void updateFlock(float dt) override;
	void updatePlanets(const PlanetManager& planet_manager) override;
	void drawAll(const TAGShaderManager::Shader& shader) override;
private:
	TAGMesh boid_mesh{ MESH_VERTICES, MESH_INDICES, MESH_MAT };
	TAGShaderManager shaders{};
	PingPongBuffer<TAGModel::ShaderObject> boids_instance;
	PingPongBuffer<BoidObject> boids_data;
	TAGResourceManager::UnmappedBuffer<BoidConstants> boids_constants;
	TAGResourceManager::UnmappedBuffer<PlanetConstants> planets_constants;
	TAGResourceManager::UnmappedBuffer<PlanetManager::PlanetData> planets;
};
