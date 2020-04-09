#define NUM_LIGHTS 10

#ifdef GL_ES
precision highp float;
#endif

uniform vec2 positions[NUM_LIGHTS];
uniform vec4 colors[NUM_LIGHTS];
uniform float radii[NUM_LIGHTS];
uniform vec2 camera_pos;
uniform int dither_buffer[64];
uniform vec3 ambient;

float get_dither_buffer(int idx) {
  for (int i=0; i<64; i++) {
    if (i == idx) return float(dither_buffer[i]);
  }
  return 0.0;
}
float get_dither(vec2 xy) {
  int x = int(mod(floor(abs(xy.x)), 8.0));
  int y = int(mod(floor(abs(xy.y)), 8.0));
  return (get_dither_buffer(x * 8 + y) - 32.0)/32.0;
}

#define STEPS 4.0
#define CENTER_PERCENT 0.8

float get_mask(float d, float radius) {
  float value = 0.0;
  float center_percent = CENTER_PERCENT;
  if (d < radius*center_percent) {
    return 1.0;
  }
  float shifted_d = max(0.0, d-radius*center_percent);
  value = 1.0 - (shifted_d / (radius * (1.0-center_percent)));

  float cur_step = floor(value / (1.0/STEPS));
  value = cur_step * 1.0/STEPS;
  //value = pow(value, 2.0);

  return value;
}

#define PI 3.1415926

vec4 effect(vec4 color, sampler2D image, vec2 uv, vec2 screen_coords) {
  vec2 pixel_pos = screen_coords;

  vec3 light_color = vec3(0,0,0);
  float mask = 0.0;

  for (int i = 0; i < NUM_LIGHTS; i++) {
    vec2 pos = positions[i] - camera_pos;
    vec4 col = colors[i];
    if (col.a == 0.0)
      continue;
    float dist = length(pos - pixel_pos);
    float radius = radii[i];
    if (radius == 0.0)
      continue;

    float dither = get_dither(pixel_pos - pos);
    float dither_dist = dist - floor(dither * radius / STEPS / 4.0);
    if (dither_dist < radius)
      mask += get_mask(dither_dist, radius);

    light_color += col.rgb * (cos(min(dist / 1.2, radius) / radius  * PI) * 0.5 + 0.5) * col.a;
  }
  mask = min(mask, 1.0);
  light_color *= mask;

  light_color = max(light_color, ambient);

  return texture2D(image, uv) * vec4(light_color, 1);
}


varying vec2 interp_uv;
varying vec4 interp_color;
uniform sampler2D ps_texture;
uniform vec2 screen_size;

void main() {
  gl_FragColor = interp_color * effect(interp_color, ps_texture, interp_uv, interp_uv*screen_size);
}


