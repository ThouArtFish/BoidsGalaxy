#version 460 core

#define WORK_GROUP_SIZE 128u

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

struct BoidConstants {
  float separation_factor, alignment_factor, cohesion_factor;
  float protected_range_sqr, visible_range_sqr;
  float border_factor;
  float min_speed, max_speed;
  vec3 max_margin;
  uint total_boids;
  float dt;
};
struct BoidInstance {
  vec3 position;
  float scale;
  vec3 axis;
  float angle;
};
struct BoidData {
  vec3 direction;
  float speed;
};
struct Boid {
  vec3 position;
  float scale;
  vec3 direction;
  float speed;
};
struct PlanetConstants {
  float orbit_pull;
  float orbit_pull_range;
  float tangent_boost;
  uint total_planets;
};
struct PlanetData {
  vec3 position;
  float orbit_radius;
};

layout(std430, binding = 0) readonly buffer inst_boids_in {
  BoidInstance[] read_inst_boids;
};
layout(std430, binding = 1) writeonly buffer inst_boids_out {
  BoidInstance[] write_inst_boids;
};
layout(std430, binding = 2) readonly buffer data_boids_in {
  BoidData[] read_data_boids;
};
layout(std430, binding = 3) writeonly buffer data_boids_out {
  BoidData[] write_data_boids;
};
layout(std140, binding = 4) uniform boids_constants {
  BoidConstants b_constants;
};
layout(std430, binding = 5) readonly buffer planet_data {
  PlanetData[] planets;
};
layout(std140, binding = 6) uniform planet_constants {
  PlanetConstants p_constants;
};

shared Boid tile[64];

vec4 pointDirection(vec3 new_dir) {
  const vec3 axis = cross(vec3(0.0, 1.0, 0.0), new_dir);
  const float axis_length = length(axis);

  return vec4((axis_length > 0.0 ? axis / axis_length : vec3(1.0, 0.0, 0.0)),
              atan(axis_length, new_dir.y));
}

Boid createBoid(BoidInstance inst, BoidData data) {
  return Boid(inst.position, inst.scale, data.direction, data.speed);
}

void main() {
  const uint i = gl_GlobalInvocationID.x;
  const uint li = gl_LocalInvocationID.x;
  const uint total_boids = b_constants.total_boids;
  const bool valid = i < total_boids;

  Boid self;
  if (valid)
    self = createBoid(read_inst_boids[i], read_data_boids[i]);
  float visible_boids = 0.0f;
  vec3 orbit_sum = vec3(0.0), repel_sum = vec3(0.0), vel_sum = vec3(0.0),
       pos_sum = vec3(0.0);

  for (uint base = 0; base < total_boids; base += WORK_GROUP_SIZE) {
    const uint j = base + li;
    if (j < total_boids)
      tile[li] = createBoid(read_inst_boids[j], read_data_boids[j]);

    memoryBarrierShared();
    barrier();

    if (valid) {
      const uint count = min(WORK_GROUP_SIZE, total_boids - base);

      for (uint k = 0; k < count; k++) {
        if (k + base == i)
          continue;

        const Boid other = tile[k];
        const vec3 dist = self.position - other.position;
        const float length_sqr = dot(dist, dist);

        if (length_sqr < b_constants.visible_range_sqr) {
          vel_sum += other.direction * other.speed;
          pos_sum += other.position;
          visible_boids += 1.0f;

          if (length_sqr < b_constants.protected_range_sqr)
            repel_sum += dist;
        }
      }
    }

    barrier();
  }

  if (!valid)
    return;

  for (uint planet_index = 0; planet_index < p_constants.total_planets;
       planet_index++) {
    PlanetData planet = planets[planet_index];

    vec3 normal = self.position - planet.position;
    const float normal_length = length(normal);

    if (normal_length > planet.orbit_radius + p_constants.orbit_pull_range)
      continue;

    normal /= normal_length;

    vec3 tangent = self.direction - normal * dot(self.direction, normal);
    if (dot(tangent, tangent) < 1e-5f) {
      tangent =
          vec3(0.0f, 1.0f, 0.0f) * (self.direction.y <= 0.0f ? 1.0f : -1.0f);
    } else {
      tangent = normalize(tangent);
    }

    const vec3 desired =
        tangent * self.speed * p_constants.tangent_boost +
        normal * p_constants.orbit_pull * (planet.orbit_radius - normal_length);

    orbit_sum += desired - self.direction * self.speed;
  }

  vec3 new_vel =
      self.direction * self.speed +
      (repel_sum * b_constants.separation_factor +
       (clamp(self.position, -b_constants.max_margin, b_constants.max_margin) -
        self.position) *
           b_constants.border_factor +
       orbit_sum) *
          b_constants.dt;
  if (visible_boids > 0.0) {
    new_vel += (((vel_sum / visible_boids) - self.direction * self.speed) *
                    b_constants.alignment_factor +
                ((pos_sum / visible_boids) - self.position) *
                    b_constants.cohesion_factor) *
               b_constants.dt;
  }

  float speed = length(new_vel);
  const vec3 dir = (speed > 0.0 ? new_vel / speed : vec3(0.0, 1.0, 0.0));
  speed = clamp(speed, b_constants.min_speed, b_constants.max_speed);
  const vec4 axis_and_angle = pointDirection(dir);
  write_inst_boids[i] =
      BoidInstance(self.position + dir * speed * b_constants.dt, self.scale,
                   vec3(axis_and_angle), axis_and_angle.w);
  write_data_boids[i] = BoidData(dir, speed);
}
