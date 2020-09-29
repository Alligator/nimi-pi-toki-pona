#include "imgui.h"
#include <SDL.h>

typedef enum {
	RESULT_NOP,
	RESULT_QUIT,
} event_result;

// these functions need to be implemented
void init();
event_result handle_event(SDL_Event event);
void setup(SDL_Window *win, ImGuiIO& io);
void loop();
void shutdown();

void set_width(int w, int h);