#include "core.h"

static bool keys_down[KEY_MAX_];
static bool keys_delta[KEY_MAX_];
static bool quit_flag;
static int width;
static int height;
static double last_time;
static float delta;

bool app_keypress(int key) { return keys_down[key] && keys_delta[key]; }
bool app_keyrelease(int key) { return !keys_down[key] && keys_delta[key]; }
bool app_keydown(int key) { return keys_down[key]; }
void app_quit(void) { quit_flag = true; }
float app_delta(void) { 
#ifdef DEV_BUILD
  if (app_keydown(KEY_LSHIFT)) {
    return delta * 5;
  }
#endif
  return delta; 
}

#include <stdio.h>
#include <stdbool.h>

static void on_keydown(int key) {
  if (key) {
    keys_down[key] = true;
    keys_delta[key] = true;
  }
}

static void on_keyup(int key) {
  if (key) {
    keys_down[key] = false;
    keys_delta[key] = true;
  }
}

static void reset_input(void) {
  memset(keys_delta, 0, sizeof(keys_delta));
}

void gfx_load(void);

//-- Emscripten -----------------------------------------------------------

#if defined(__EMSCRIPTEN__)

#include <emscripten/html5.h>
#include <emscripten/emscripten.h>

double app_time(void) {
  return emscripten_get_now() / 1000.0;
}

void (*main_frame)(void);
static void em_main_loop(void) {

  double now = app_time();
  delta = (float)(now - last_time);
  last_time = now;

  if (main_frame)
    main_frame();

  reset_input();
}

int em_translate_key(int key) {
  if (key >= 65 && key <= 65+'z'-'a') {
    return ((key-65) + KEY_A);
  }
  if (key >= 48 && key <= 48+'0'-'9') {
    return ((key-48) + KEY_0);
  }
  switch (key) {
    case 37: return KEY_LEFT;
    case 39: return KEY_RIGHT;
    case 38: return KEY_UP;
    case 40: return KEY_DOWN;
    case 16: return KEY_LSHIFT;
    //case 32: return KEY_SPACE;
    //case 189: return KEY_MINUS;
    //case 187: return KEY_EQUAL;
  }
  return 0;
}

EM_BOOL em_key_down(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
  if (!keyEvent->repeat) {
    int k = em_translate_key(keyEvent->keyCode);
    if (k) {
      on_keydown(k);
    }
  }
  return true;
}
EM_BOOL em_key_up(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
  int k = em_translate_key(keyEvent->keyCode);
  if (k) {
    on_keyup(k);
  }
  return true;
}

void app_start(App_Config *conf) {
  width = conf->width ? conf->width : 800;
  height = conf->height ? conf->height : 600;

  const char *id = "#canvas";

  emscripten_set_canvas_element_size(id, width, height);

  {
    EmscriptenWebGLContextAttributes attr;
    attr.alpha = EM_TRUE;
    attr.depth = EM_TRUE;
    attr.stencil = EM_FALSE;
    attr.antialias = EM_TRUE;
    attr.premultipliedAlpha = EM_TRUE;
    attr.preserveDrawingBuffer = EM_TRUE;
    attr.preferLowPowerToHighPerformance = EM_TRUE;
    attr.failIfMajorPerformanceCaveat = EM_FALSE;
    attr.majorVersion = 1;
    attr.minorVersion = 0;
    attr.enableExtensionsByDefault = EM_FALSE;
    int ctx = emscripten_webgl_create_context(id, &attr);
    emscripten_webgl_make_context_current(ctx);
    //emscripten_set_resize_callback(0, 0, EM_TRUE, &hb_on_resize_);
  }

  emscripten_set_keydown_callback("#window", NULL, true, &em_key_down);
  emscripten_set_keyup_callback("#window", NULL, true, &em_key_up);

  gfx_width = width;
  gfx_height = height;
  gfx_load();

  if (conf->load)
    conf->load();

  main_frame = conf->step;

  emscripten_set_main_loop(em_main_loop, 0, true);
}

//-- GLFW3 -----------------------------------------------------------

#else

#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

double app_time(void) {
  return glfwGetTime();
}

void (*main_frame)(void);
static void glfw_main_loop(void) {

  double now = app_time();
  delta = (float)(now - last_time);
  last_time = now;

  if (main_frame)
    main_frame();

  reset_input();
}

int glfw_translate_key(int key) {
  if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
    return ((key-GLFW_KEY_A) + KEY_A);
  }
  if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
    return ((key-GLFW_KEY_0) + KEY_0);
  }
  switch (key) {
    case GLFW_KEY_LEFT:       return KEY_LEFT;
    case GLFW_KEY_RIGHT:      return KEY_RIGHT;
    case GLFW_KEY_UP:         return KEY_UP;
    case GLFW_KEY_DOWN:       return KEY_DOWN;
    case GLFW_KEY_LEFT_SHIFT: return KEY_LSHIFT;
  }
  return 0;
}

void glfw_keycb(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void) window; (void) scancode; (void) mods;
	int k = glfw_translate_key(key);
	if (action == GLFW_PRESS)
	{
		on_keydown(k);
		return;
	}
	if (action == GLFW_RELEASE)
	{
		on_keyup(k);
		return;
	}
}

void glfw_errorcb(int errid, const char *errtext)
{
	(void) errid;
	fprintf(stderr, "Error: %s\n", errtext);
}

void app_start(App_Config *conf) {
	width = conf->width ? conf->width : 800;
	height = conf->height ? conf->height : 600;
	const char *name = conf->title ? conf->title : "untitled";

	glfwInit();
	glfwSetErrorCallback(glfw_errorcb);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API) ;
	GLFWwindow *window = glfwCreateWindow(width, height, name, NULL, NULL);
	glfwSetKeyCallback(window, glfw_keycb);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	//initgl();

	gfx_width = width;
	gfx_height = height;
	gfx_load();

	if (conf->load)
	conf->load();

	main_frame = conf->step;

	while (!glfwWindowShouldClose(window))
	{
		//int width, height;
		//glfwGetFramebufferSize(window, &width, &height);
		glfw_main_loop();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
}

#endif

