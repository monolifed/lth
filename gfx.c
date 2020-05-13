#include "core.h"

//==================== Matrix math ======================
static void matrix_identity(float t[16]) {
  t[0]  = 1; t[1]  = 0; t[2]  = 0; t[3]  = 0;
  t[4]  = 0; t[5]  = 1; t[6]  = 0; t[7]  = 0;
  t[8]  = 0; t[9]  = 0; t[10] = 1; t[11] = 0;
  t[12] = 0; t[13] = 0; t[14] = 0; t[15] = 1;
}
static void matrix_translate(float t[16], float tx, float ty) {
  t[12] += t[0]*tx + t[4]*ty;
  t[13] += t[1]*tx + t[5]*ty;
}
static void matrix_scale(float t[16], float sx, float sy) {
  t[0] *= sx;
  t[4] *= sy;
  t[1] *= sx;
  t[5] *= sy;
}
static void matrix_rotate(float t[16], float r) {
  r = r / 180 * M_PI;
  float Sin = sinf(r);
  float Cos = cosf(r);
  float t0 = t[0];
  float t4 = t[4];
  float t1 = t[1];
  float t5 = t[5];
  t[0] = t0* Cos + t4*Sin;
  t[4] = t0*-Sin + t4*Cos;
  t[1] = t1* Cos + t5*Sin;
  t[5] = t1*-Sin + t5*Cos;
}
static void matrix_multiply(float t[16], const float a[16], const float b[16]) {
  t[0 ] = a[0 ]*b[0 ] + a[4 ]*b[1 ] + b[8 ]*b[2 ] + a[12]*b[3 ];
  t[1 ] = a[1 ]*b[0 ] + a[5 ]*b[1 ] + b[9 ]*b[2 ] + a[13]*b[3 ];
  t[2 ] = a[2 ]*b[0 ] + a[6 ]*b[1 ] + b[10]*b[2 ] + a[14]*b[3 ];
  t[3 ] = a[3 ]*b[0 ] + a[7 ]*b[1 ] + b[11]*b[2 ] + a[15]*b[3 ];

  t[4 ] = a[0 ]*b[4 ] + a[4 ]*b[5 ] + b[8 ]*b[6 ] + a[12]*b[7 ];
  t[5 ] = a[1 ]*b[4 ] + a[5 ]*b[5 ] + b[9 ]*b[6 ] + a[13]*b[7 ];
  t[6 ] = a[2 ]*b[4 ] + a[6 ]*b[5 ] + b[10]*b[6 ] + a[14]*b[7 ];
  t[7 ] = a[3 ]*b[4 ] + a[7 ]*b[5 ] + b[11]*b[6 ] + a[15]*b[7 ];

  t[8 ] = a[0 ]*b[8 ] + a[4 ]*b[9 ] + b[8 ]*b[10] + a[12]*b[11];
  t[9 ] = a[1 ]*b[8 ] + a[5 ]*b[9 ] + b[9 ]*b[10] + a[13]*b[11];
  t[10] = a[2 ]*b[8 ] + a[6 ]*b[9 ] + b[10]*b[10] + a[14]*b[11];
  t[11] = a[3 ]*b[8 ] + a[7 ]*b[9 ] + b[11]*b[10] + a[15]*b[11];

  t[12] = a[0 ]*b[12] + a[4 ]*b[13] + b[8 ]*b[14] + a[12]*b[15];
  t[13] = a[1 ]*b[12] + a[5 ]*b[13] + b[9 ]*b[14] + a[13]*b[15];
  t[14] = a[2 ]*b[12] + a[6 ]*b[13] + b[10]*b[14] + a[14]*b[15];
  t[15] = a[3 ]*b[12] + a[7 ]*b[13] + b[11]*b[14] + a[15]*b[15];
}
static float *get_projection(float *t, int is_upright, int width, int height) {
  memset(t, 0, 16 * sizeof(float));
  int upright = is_upright ? 1 : -1;

  t[0] = 1;
  t[5] = 1;
  t[10] = 1;
  t[15] = 1;

  t[12] = -1;
  t[13] = 1;
  t[14] = 0;
  float a = 1.0f / ((float)width / 2);
  float b = -1.0f / ((float)height / 2) * upright;
  t[0] = a;
  t[5] = b;
  t[13] = (float)upright;
  return t;
}

#if defined(__EMSCRIPTEN__)
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#include <GLES2/gl2.h>


static void gfx_load_gl_functions(void) {}

#else

#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

// Note:
#define REG_GLXT_WIN32 \
GLXT(PFNGLACTIVETEXTUREPROC,glActiveTexture) \

#define REG_GLXT \
GLXT(PFNGLBLENDFUNCSEPARATEPROC,glBlendFuncSeparate) \
GLXT(PFNGLBINDFRAMEBUFFERPROC,glBindFramebuffer) \
GLXT(PFNGLGENBUFFERSPROC,glGenBuffers) \
GLXT(PFNGLBINDBUFFERPROC,glBindBuffer) \
GLXT(PFNGLBUFFERDATAPROC,glBufferData) \
GLXT(PFNGLBUFFERSUBDATAPROC,glBufferSubData) \
GLXT(PFNGLUSEPROGRAMPROC,glUseProgram) \
GLXT(PFNGLCREATESHADERPROC,glCreateShader) \
GLXT(PFNGLSHADERSOURCEPROC,glShaderSource) \
GLXT(PFNGLCOMPILESHADERPROC,glCompileShader) \
GLXT(PFNGLGETSHADERIVPROC,glGetShaderiv) \
GLXT(PFNGLGETSHADERINFOLOGPROC,glGetShaderInfoLog) \
GLXT(PFNGLVERTEXATTRIBPOINTERPROC,glVertexAttribPointer) \
GLXT(PFNGLENABLEVERTEXATTRIBARRAYPROC,glEnableVertexAttribArray) \
GLXT(PFNGLDELETESHADERPROC,glDeleteShader) \
GLXT(PFNGLATTACHSHADERPROC,glAttachShader) \
GLXT(PFNGLLINKPROGRAMPROC,glLinkProgram) \
GLXT(PFNGLCREATEPROGRAMPROC,glCreateProgram) \
GLXT(PFNGLGETPROGRAMIVPROC,glGetProgramiv) \
GLXT(PFNGLGETPROGRAMINFOLOGPROC,glGetProgramInfoLog) \
GLXT(PFNGLDELETEPROGRAMPROC,glDeleteProgram) \
GLXT(PFNGLGETATTRIBLOCATIONPROC,glGetAttribLocation) \
GLXT(PFNGLGETACTIVEUNIFORMPROC,glGetActiveUniform) \
GLXT(PFNGLGETUNIFORMLOCATIONPROC,glGetUniformLocation) \
GLXT(PFNGLGENFRAMEBUFFERSPROC,glGenFramebuffers) \
GLXT(PFNGLFRAMEBUFFERTEXTURE2DPROC,glFramebufferTexture2D) \
GLXT(PFNGLUNIFORM1FVPROC,glUniform1fv) \
GLXT(PFNGLUNIFORM2FVPROC,glUniform2fv) \
GLXT(PFNGLUNIFORM4FVPROC,glUniform4fv) \
GLXT(PFNGLUNIFORM1IPROC,glUniform1i) \
GLXT(PFNGLUNIFORMMATRIX4FVPROC,glUniformMatrix4fv) \
GLXT(PFNGLDEBUGMESSAGECALLBACKPROC,glDebugMessageCallback) \
GLXT(PFNGLUNIFORM3FVPROC,glUniform3fv) \
GLXT(PFNGLUNIFORM1IVPROC,glUniform1iv)


#define GLXT(type, name) type name = NULL;
#ifdef _WIN32
REG_GLXT_WIN32
#endif

REG_GLXT
#undef GLXT

static void gl_message_callback(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar *message,
  const void *userParam)
{
	(void) source; (void) type; (void) id; (void) length; (void) userParam;
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		printf("%s\n", message);
}

GLFWglproc gl_get_ext(const char *procname)
{
	GLFWglproc fn = glfwGetProcAddress(procname);
	if (!fn)
	{
		fprintf(stderr, "failed getting proc address: %s", procname);
	}
	return fn;
}


static void gfx_load_gl_functions(void)
{
#define GLXT(type, name) name = (type) gl_get_ext(#name);
	#ifdef _WIN32
	REG_GLXT_WIN32
	#endif
	
	REG_GLXT
#undef GLXT
}

#endif

typedef struct Vertex {
  float pos[2];
  float uv[2];
  uint32_t color;
} Vertex;

typedef struct Matrix4 {
  float data[16];
} Matrix4;

static Shader *default_shader;
static Shader *custom_shader;
static Texture *dummy_texture;
static Canvas *cur_canvas;
static bool use_scissor;
static float scissor[4];
static GLuint imm_handle;
static int imm_capacity;
static int imm_cpu_size;
static int imm_cpu_capacity;
static Vertex *imm_cpu_buf;
static float color[4] = {1.f, 1.f, 1.f, 1.f};
static Matrix4 mat_stack[256];
static unsigned mat_cursor;

int gfx_width;
int gfx_height;

void gfx_set_color(float r, float g, float b, float a) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
}

void gfx_get_color(float out_color[4]) {
  out_color[0] = color[0];
  out_color[1] = color[1];
  out_color[2] = color[2];
  out_color[3] = color[3];
}

uint32_t gfx_get_colori(void) {
  return
    ((uint32_t)(color[3] * 255.0f) << 24) |
    ((uint32_t)(color[2] * 255.0f) << 16) |
    ((uint32_t)(color[1] * 255.0f) << 8 ) |
    ((uint32_t)(color[0] * 255.0f) << 0 );
}

void gfx_set_scissor(float x, float y, float w, float h) {
  use_scissor = true;
  scissor[0] = x;
  scissor[1] = y;
  scissor[2] = w;
  scissor[3] = h;
  glEnable(GL_SCISSOR_TEST);
}

void gfx_unset_scissor(void) {
  use_scissor = false;
  glDisable(GL_SCISSOR_TEST);
}

static Texture gfx_make_texture_impl(const void *data, int width, int height) {
  Texture t = {.handle = 0, .width = width, .height = height};
  
  glGenTextures(1, &t.handle);
  glBindTexture(GL_TEXTURE_2D, t.handle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);


  return t;
}

Texture *gfx_make_texture(const void *data, int width, int height) {
  Texture t = gfx_make_texture_impl(data, width, height);
  Texture *result = calloc(1, sizeof t);
  memcpy(result, &t, sizeof t);
  return result;
}

void gfx_delete_texture(Texture *t) {
  GLuint handle = (GLuint)t->handle;
  glDeleteTextures(1, &handle);
  free(t);
}

static GLuint gfx_make_gl_shader(GLenum type, const char *src, int size) {
  GLuint handle = glCreateShader(type);
  glShaderSource(handle, 1, &src, &size);
  glCompileShader(handle);

  GLint status = 0;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
  if (!status) {
    GLint length = 0;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length);
    char *buffer = malloc(length);
    glGetShaderInfoLog(handle, length, NULL, buffer);
    printf("Failed to compile shader:\n%s\n", buffer);
    printf("%.*s\n", size, src);
    free(buffer);
    glDeleteShader(handle);
    return 0;
  }

  return handle;
}

static Shader *gfx_make_shader_impl(const char *vs_src, int vs_size, const char *ps_src, int ps_size) {
  GLuint vs = 0;
  GLuint ps = 0;
  GLuint handle = 0;

  vs = gfx_make_gl_shader(GL_VERTEX_SHADER, vs_src, vs_size);
  if (vs) {
    ps = gfx_make_gl_shader(GL_FRAGMENT_SHADER, ps_src, ps_size);
    if (ps) {

      handle = glCreateProgram();
      glAttachShader(handle, vs);
      glAttachShader(handle, ps);
      glLinkProgram(handle);

      GLint status = 0;
      glGetProgramiv(handle, GL_LINK_STATUS, &status);
      if (!status) {
        GLint length = 0;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
        char *buffer = malloc(length);
        glGetProgramInfoLog(handle, length, NULL, buffer);
        printf("%s\n", buffer);
        free(buffer);
        glDeleteProgram(handle);
        handle = 0;
      }
    }
  }

  if (vs) glDeleteShader(vs);
  if (ps) glDeleteShader(ps);

  Shader *s = calloc(1, sizeof *s);
  s->handle = handle;
  return s;
}

Shader *gfx_make_shader(const char *src, size_t size) {
  const char vs[] = 
    "#ifdef GL_ES\n"
    "precision highp float;\n"
    "#endif\n"
    "attribute vec2 vs_pos;"
    "attribute vec2 vs_uv;"
    "attribute vec4 vs_color;"

    "uniform mat4 vs_matrix;"

    "varying vec2 interp_uv;"
    "varying vec4 interp_color;"

    "void main() {"
    "  interp_uv = vs_uv;"
    "  interp_color = vs_color;"
    "  gl_Position = vs_matrix * vec4(vs_pos,0,1);"
    "  gl_PointSize = 1.0;"
    "}"
  ;
  return gfx_make_shader_impl(vs, sizeof(vs)-1, src, (int)size);
}

void gfx_push(void) {
  assert(mat_cursor+1 < array_count(mat_stack) && "Matrix stack full");
  mat_cursor++;
  mat_stack[mat_cursor] = mat_stack[mat_cursor-1];
}
void gfx_pop(void) {
  assert(mat_cursor > 0 && "Matrix stack empty");
  mat_cursor--;
}
void gfx_translate(float tx, float ty) {
  matrix_translate(mat_stack[mat_cursor].data, tx, ty);
}
void gfx_scale(float sx, float sy) {
  matrix_scale(mat_stack[mat_cursor].data, sx, sy);
}
void gfx_rotate(float r) {
  matrix_rotate(mat_stack[mat_cursor].data, r);
}


void gfx_set_shader(Shader *shader) {
  custom_shader = shader;
  if (shader) {
    glUseProgram((GLuint)shader->handle);
  }
  else {
    glUseProgram((GLuint)default_shader->handle);
  }
}

void gfx_shader_send(const char *name, int type, const void *data, size_t count) {
  if (!custom_shader) {
    return;
  }

  GLint loc = glGetUniformLocation((GLuint)custom_shader->handle, name);
  if (loc == -1) {
    return;
  }

  switch (type) {
    default:
      break;
    case GFX_UNIFORM_FLOAT:
      glUniform1fv(loc, count, (float *)data);
      break;
    case GFX_UNIFORM_FLOAT2:
      glUniform2fv(loc, count, (float *)data);
      break;
    case GFX_UNIFORM_FLOAT3:
      glUniform3fv(loc, count, (float *)data);
      break;
    case GFX_UNIFORM_FLOAT4:
      glUniform4fv(loc, count, (float *)data);
      break;
    //case GFX_UNIFORM_UINT:
    //  glUniform1uiv(loc, count, (uint32_t *)data);
    //  break;
    case GFX_UNIFORM_INT:
      glUniform1iv(loc, count, (int32_t *)data);
      break;
  }
}

static void gfx_imm_draw_impl(Texture *texture, GLenum primitive) {
  if (!texture)
    texture = dummy_texture;

  GLuint program = (GLuint)default_shader->handle;
  if (custom_shader)
    program = (GLuint)custom_shader->handle;

  glBindBuffer(GL_ARRAY_BUFFER, imm_handle);

  GLint loc = -1;
  loc = glGetAttribLocation(program, "vs_pos");
  if (loc != -1) {
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
    glEnableVertexAttribArray(loc);
  }
  loc = glGetAttribLocation(program, "vs_uv");
  if (loc != -1) {
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(float)*2));
    glEnableVertexAttribArray(loc);
  }
  loc = glGetAttribLocation(program, "vs_color");
  if (loc != -1) {
    glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)(sizeof(float)*4));
    glEnableVertexAttribArray(loc);
  }

  int width = cur_canvas ? cur_canvas->width : gfx_width;
  int height = cur_canvas ? cur_canvas->height : gfx_height;
  int upright = !cur_canvas ? 1 : 0;

  if (use_scissor) {
    GLint y = height - (GLint)(scissor[3]+scissor[1]);
    if (!upright) {
      y = (GLint)scissor[1];
    }
    glScissor((GLint)scissor[0], y,
      (GLint)scissor[2], (GLint)scissor[3]);
  }

  float projection[16];
  get_projection(projection, upright, width, height);

  float matrix[16];
  matrix_multiply(matrix, projection, mat_stack[mat_cursor].data);
  glUniformMatrix4fv(glGetUniformLocation(program, "vs_matrix"), 1, GL_FALSE, matrix);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, (GLuint)texture->handle);
  glUniform1i(glGetUniformLocation(program, "ps_texture"), 0);

  glDrawArrays(primitive, 0, (GLsizei)imm_cpu_size);
}

void gfx_clear(float r, float g, float b, float a) {
  glClearColor(r,g,b,a);
  glClear(GL_COLOR_BUFFER_BIT);
}

void gfx_fill_rect(float x, float y, float w, float h) {
  uint32_t color = gfx_get_colori();
  gfx_imm_begin();
  gfx_imm_vertex(x,    y,    0,0, color);
  gfx_imm_vertex(x+w,  y,    1,0, color);
  gfx_imm_vertex(x+w,  y+h,  1,1, color);

  gfx_imm_vertex(x,    y,    0,0, color);
  gfx_imm_vertex(x+w,  y+h,  1,1, color);
  gfx_imm_vertex(x,    y+h,  0,1, color);
  gfx_imm_end();

  gfx_imm_draw_impl(NULL, GL_TRIANGLES);
}

void gfx_trace_rect(float x, float y, float w, float h) {
  gfx_fill_rect(x,y, w, 1);
  gfx_fill_rect(x,y, 1, h);
  gfx_fill_rect(x,y+h-1, w, 1);
  gfx_fill_rect(x+w-1,y, 1, h);
}

void gfx_line(float x0, float y0, float x1, float y1) {
  uint32_t c = gfx_get_colori();
  gfx_imm_begin();
  gfx_imm_vertex(  x0,  y0,    0,0, c);
  gfx_imm_vertex(  x1,  y1,    1,0, c);
  gfx_imm_end();
  gfx_imm_draw_impl(NULL, GL_LINES);
}

void gfx_imm_begin(void) {
  imm_cpu_size = 0;
}

void gfx_imm_vertex(float x, float y, float u, float v, uint32_t color) {
  Vertex vertex;
  vertex.pos[0] = x;
  vertex.pos[1] = y;
  vertex.uv[0] = u;
  vertex.uv[1] = v;
  vertex.color = color;

  if (imm_cpu_size >= imm_cpu_capacity) {
    imm_cpu_capacity = imm_cpu_capacity ? 
        imm_cpu_capacity * 2 : 1024;

    imm_cpu_buf = realloc(imm_cpu_buf,
        imm_cpu_capacity * sizeof *imm_cpu_buf);
  }

  imm_cpu_buf[imm_cpu_size++] = vertex;
}

void gfx_imm_end(void) {
  if (!imm_handle) {
    glGenBuffers(1, &imm_handle);
  }
  if (imm_cpu_size > imm_capacity) {
    imm_capacity = max(1024, imm_cpu_size);
    glBindBuffer(GL_ARRAY_BUFFER, imm_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * imm_capacity, NULL, GL_DYNAMIC_DRAW);
  }

  glBindBuffer(GL_ARRAY_BUFFER, imm_handle);
  glBufferSubData(GL_ARRAY_BUFFER, 0, imm_cpu_size * sizeof(Vertex), imm_cpu_buf);
}

void gfx_draw_textureq(Texture *t, float quad[4]) {
  float w = quad[2];
  float h = quad[3];

  float u0 = quad[0] / t->width;
  float v0 = quad[1] / t->height;
  float u1 = (quad[0]+quad[2]) / t->width;
  float v1 = (quad[1]+quad[3]) / t->height;

  uint32_t color = gfx_get_colori();
  gfx_imm_begin();
  gfx_imm_vertex(0,    0,    u0,v0, color);
  gfx_imm_vertex(0+w,  0,    u1,v0, color);
  gfx_imm_vertex(0+w,  0+h,  u1,v1, color);

  gfx_imm_vertex(0,    0,    u0,v0, color);
  gfx_imm_vertex(0+w,  0+h,  u1,v1, color);
  gfx_imm_vertex(0,    0+h,  u0,v1, color);
  gfx_imm_end();
  gfx_imm_draw_impl(t, GL_TRIANGLES);
}

void gfx_draw_texture(Texture *t) {
  float quad[4] = {0.f, 0.f, (float)t->width, (float)t->height};
  gfx_draw_textureq(t, quad);
}

Canvas *gfx_make_canvas(int width, int height) {
  GLuint handle = 0;
  glGenFramebuffers(1, &handle);
  glBindFramebuffer(GL_FRAMEBUFFER, handle);
  Texture t = gfx_make_texture_impl(NULL, width, height);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)t.handle, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  Canvas *c = calloc(1, sizeof *c);
  c->handle = handle;
  c->texture = t;
  c->width = width;
  c->height = height;
  return c;
}

void gfx_set_canvas(Canvas *canvas) {
  cur_canvas = canvas;
  if (canvas) {
    glViewport(0, 0, canvas->width, canvas->height);
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)canvas->handle);
  }
  else {
    glViewport(0, 0, gfx_width, gfx_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void gfx_set_blend_mode(int blend_mode) {
  switch (blend_mode) {
    case GFX_BLEND_ALPHA:
      glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case GFX_BLEND_MULTIPLY:
      glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
      break;
  }
}

void gfx_load(void) {
  gfx_load_gl_functions();
#ifdef GL_DEBUG_OUTPUT
  if (glDebugMessageCallback) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_message_callback, 0);
  }
#endif
  glEnable(GL_BLEND);
  gfx_set_blend_mode(GFX_BLEND_ALPHA);

  matrix_identity(mat_stack[0].data);

  const char ps[] =
    "#ifdef GL_ES\n"
    "precision highp float;\n"
    "#endif\n"
    "uniform sampler2D ps_texture;"

    "varying vec2 interp_uv;"
    "varying vec4 interp_color;"

    "void main() {"
    "  vec4 tex_color = interp_color * texture2D(ps_texture, interp_uv);"
    "  gl_FragColor = vec4(tex_color.rgb/* * (1.0-ps_color_override) +"
    "                 ps_color_override * ps_override_color.rgb*/, tex_color.a);"
    "}"
  ;

  default_shader = gfx_make_shader(ps, sizeof(ps)-1);

  uint32_t color = 0xffffffffU;
  dummy_texture = gfx_make_texture(&color, 1,1);

  gfx_set_shader(0);

  glViewport(0,0,gfx_width, gfx_height);
}

