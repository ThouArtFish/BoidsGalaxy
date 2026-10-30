#version 460 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct Constants {
  float separation_factor, alignment_factor, cohesion_factor;
  float protected_range_sqr, visible_range_sqr;
  float border_factor;
  float min_speed, max_speed;
  uint total_boids;
  vec3 max_margin;
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
layout(std140, binding = 4) readonly uniform boids_constants {
  Constants constants;
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
  const uint total_boids = constants.total_boids;
  const bool valid = i < total_boids;

  Boid self;
  if (valid)
    self = createBoid(read_inst_boids[i], read_data_boids[i]);
  float visible_boids = 0.0f;
  vec3 repel_sum = vec3(0.0), vel_sum = vec3(0.0), pos_sum = vec3(0.0);

  for (uint base = 0; base < total_boids; base += 64u) {
    const uint j = base + li;
    if (j < total_boids)
      tile[li] = createBoid(read_inst_boids[j], read_data_boids[j]);

    memoryBarrierShared();
    barrier();

    if (valid) {
      const uint count = min(64u, total_boids - base);

      for (uint k = 0; k < count; k++) {
        if (k + base == i)
          continue;

        const Boid other = tile[k];
        const vec3 dist = self.position - other.position;
        const float length_sqr = dot(dist, dist);

        if (length_sqr < constants.visible_range_sqr) {
          vel_sum += other.direction * other.speed;
          pos_sum += other.position;
          visible_boids += 1.0f;

          if (length_sqr < constants.protected_range_sqr)
            repel_sum += dist;
        }
      }
    }

    barrier();
  }

  if (!valid)
    return;

  vec3 new_vel =
      self.direction * self.speed +
      (repel_sum * constants.separation_factor +
       (clamp(self.position, -constants.max_margin, constants.max_margin) -
        self.position) *
           constants.border_factor) *
          constants.dt;
  if (visible_boids > 0.0) {
    new_vel += (((vel_sum / visible_boids) - self.direction * self.speed) *
                    constants.alignment_factor +
                ((pos_sum / visible_boids) - self.position) *
                    constants.cohesion_factor) *
               constants.dt;
  }

  float speed = length(new_vel);
  const vec3 dir = (speed > 0.0 ? new_vel / speed : vec3(0.0, 1.0, 0.0));
  speed = clamp(speed, constants.min_speed, constants.max_speed);
  const vec4 axis_and_angle = pointDirection(dir);
  write_inst_boids[i] =
      BoidInstance(self.position + dir * speed * constants.dt, self.scale,
                   vec3(axis_and_angle), axis_and_angle.w);
  write_data_boids[i] = BoidData(dir, speed);
}
