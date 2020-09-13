#include "imgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "impl/impl.h"

#include "parson.h"
#include "resources/icon.h"
#include "resources/font.h"

// pico 8 colours
static ImColor col_black        = ImColor(0, 0, 0);
static ImColor col_dark_blue    = ImColor(29, 43, 83);
static ImColor col_dark_purple  = ImColor(126, 37, 83);
static ImColor col_dark_green   = ImColor(0, 135, 81);
static ImColor col_brown        = ImColor(171, 82, 54);
static ImColor col_dark_grey    = ImColor(95, 87, 79);
static ImColor col_light_grey   = ImColor(194, 195, 199);
static ImColor col_white        = ImColor(255, 241, 232);
static ImColor col_red          = ImColor(255, 0, 77);
static ImColor col_orange       = ImColor(255, 163, 0);
static ImColor col_yellow       = ImColor(255, 236, 39);
static ImColor col_green        = ImColor(0, 228, 54);
static ImColor col_blue         = ImColor(41, 173, 255);
static ImColor col_lavender     = ImColor(131, 118, 156);
static ImColor col_pink         = ImColor(255, 119, 168);
static ImColor col_light_peach  = ImColor(255, 204, 170);

JSON_Value *dictionary_root;
JSON_Object *dictionary;

static int window_width = 400;
static int window_height = 300;

struct word_def {
	const char *kind;
	const char *definition;
};

struct app_state {
	bool use_clipboard = true;
	bool error = false;
	char error_message[256];
	char *clipboard;
	char input[50];
	word_def defs[10];
	int def_count = 0;
};

app_state state;

// dictionary stuff
void dictionary_init() {
	dictionary_root = json_parse_file("dictionary.json");
	if (dictionary_root == NULL) {
		state.error = true;
		memcpy(state.error_message, "could not parse JSON!", strlen("could not parse JSON"));
		return;
	}
	dictionary = json_value_get_object(dictionary_root);
}

int dictionary_lookup(const char *word, struct word_def def[]) {
	if (state.error) {
		return 0;
	}
	JSON_Array *array = json_object_get_array(dictionary, word);
	if (array == NULL) {
		return 0;
	}
	size_t count = json_array_get_count(array);

	for (int i = 0; i < count; i++) {
		JSON_Object *val = json_array_get_object(array, i);
		struct word_def d;
		d.kind = json_object_get_string(val, "kind");
		if (d.kind == NULL) {
			state.error = true;
			snprintf(state.error_message, 256, "property missing from JSON!\n%s[%d] has no 'kind' attribute", word, i);
			return 0;
		}
		d.definition = json_object_get_string(val, "definition");
		if (d.definition == NULL) {
			state.error = true;
			snprintf(state.error_message, 256, "property missing from JSON!\n%s[%d] has no 'definition' attribute", word, i);
			return 0;
		}
		def[i] = d;
	}

	return count;
}

void dictionary_free() {
	json_value_free(dictionary_root);
}


// utils
void read_clipboard() {
	char *clipboard = SDL_GetClipboardText();

	if (clipboard == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return;
	}

	// truncate it to 50 chars max
	if (strlen(clipboard) > 50) {
		clipboard[50] = '\0';
	}

	// trim whitespace
	int end = strlen(clipboard) - 1;
	while (SDL_isspace(clipboard[end])) {
		end--;
	}
	clipboard[end + 1] = '\0';

	if (state.clipboard == NULL || strcmp(state.clipboard, clipboard) != 0) {
		if (state.clipboard != NULL) {
			SDL_free(state.clipboard);
		}
		state.clipboard = clipboard;
		state.def_count = dictionary_lookup(state.clipboard, state.defs);
	}
}

static void center_text(char *text, ImU32 color = 0) {
	ImVec2 size = ImGui::CalcTextSize(text);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size.x / 2));
	if (color == 0) {
		ImGui::Text(text);
	} else {
		ImGui::TextColored(ImColor(color), text);
	}
}

static void help_maker(char *text) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("%s", text);
	}
}


// the business
void init() {
	dictionary_init();
	set_width(window_width, window_height);
}

void setup(SDL_Window *win, ImGuiIO& io) {
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(nimi_icon, 64, 64, 32, 256, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	if (surface == NULL) {
		const char* err = SDL_GetError();
		fprintf(stderr, "%s\n", err);
		exit(0);
	}

	SDL_SetWindowIcon(win, surface);
	SDL_FreeSurface(surface);

	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;
	io.Fonts->AddFontFromMemoryTTF(nimi_font, SDL_arraysize(nimi_font), 17.0f, &config);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBgActive] = col_dark_purple;
	style.Colors[ImGuiCol_Text] = col_white;
	style.Colors[ImGuiCol_TextDisabled] = col_lavender;

	style.Colors[ImGuiCol_Button] = col_dark_purple;
	style.Colors[ImGuiCol_ButtonHovered] = col_pink;
	style.Colors[ImGuiCol_ButtonActive] = col_pink;

	style.Colors[ImGuiCol_FrameBg] = col_dark_purple;
	style.Colors[ImGuiCol_FrameBgHovered] = col_pink;
	style.Colors[ImGuiCol_FrameBgActive] = col_pink;

	style.Colors[ImGuiCol_CheckMark] = col_pink;
}

event_result handle_event(SDL_Event event) {
	if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
		window_width = event.window.data1;
		window_height = event.window.data2;
	}
	if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
		return RESULT_QUIT;
	}
	return RESULT_NOP;
}

static unsigned int prev_ticks = 0; 

void loop() {
	unsigned int cur_ticks = SDL_GetTicks();

	if (cur_ticks > prev_ticks + 250) {
		prev_ticks = cur_ticks;
		read_clipboard();
	}
	
	ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	ImGui::Begin("nimi pi toki pona", NULL,
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoBackground
	);

	if (state.error) {
		ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup("error");
		if (ImGui::BeginPopupModal("error", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::Text("%s", state.error_message);
			ImGui::EndPopup();
		}
		ImGui::End();
		return;
	}

	ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
	if (state.use_clipboard) {
		ImGui::InputText("", state.clipboard, 50, ImGuiInputTextFlags_ReadOnly);
	} else {
		if (ImGui::InputTextWithHint("", "enter word", state.input, 50)) {
			state.def_count = dictionary_lookup(state.input, state.defs);
		}
	}

	ImGui::BeginGroup();
	ImGui::BeginChild("defwin", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
	if (state.def_count > 0) {
		for (int i = 0; i < state.def_count; i++) {
			ImGui::TextColored(ImColor(col_pink), "%s", state.defs[i].kind);
			ImGui::Indent();
			ImGui::TextWrapped("%s", state.defs[i].definition);
			ImGui::Unindent();
		}
	} else {
		ImGui::TextWrapped("%s", "no definition found");
	}
	ImGui::EndChild();

	if (ImGui::RadioButton("clipboard", state.use_clipboard)) {
		state.use_clipboard = true;
		state.def_count = dictionary_lookup(state.clipboard, state.defs);
	}

	ImGui::SameLine(); help_maker("look up the word in your clipboard");

	ImGui::SameLine();
	if (ImGui::RadioButton("manual", !state.use_clipboard)) {
		state.use_clipboard = false;
		state.def_count = dictionary_lookup(state.input, state.defs);
	}

	ImGui::SameLine(); help_maker("look up a manually entered word");

	ImGui::SameLine(ImGui::GetWindowWidth()-65);
	if (ImGui::Button("about")) {
		ImGui::OpenPopup("about");
	}
	
	ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(200.0f, ImGui::GetTextLineHeightWithSpacing() * 8.75f));

	bool open = true;
	if (ImGui::BeginPopupModal("about", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
		center_text("nimi pi toki pona", col_pink); 
		center_text("toki pona words");

		ImGui::NewLine();

		center_text("ilo tan jan alikato", col_pink);
		center_text("a tool by alligator");

		ImGui::NewLine();

		center_text("alligatr.co.uk", col_blue);
		ImGui::EndPopup();
	}

	ImGui::EndGroup();

	ImGui::End();
}
