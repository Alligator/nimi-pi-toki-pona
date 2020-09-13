#include "imgui.h"
#include <SDL.h>

typedef enum {
	RESULT_NOP,
	RESULT_QUIT,
} event_result;

// forward declarations
void init();
event_result handle_event(SDL_Event event);
void setup(SDL_Window *win, ImGuiIO& io);
void loop();

void set_width(int w, int h);