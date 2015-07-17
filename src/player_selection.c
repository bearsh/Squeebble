#include "player_selection.h"

#define sfree(x)       do { if (x) free(x); } while (0)
#define nfree(x)       do { if (x) { free(x); x = NULL; } } while (0)

static SimpleMenuLayer *psel_menu_layer;
static SimpleMenuSection psel_sections = {
	.title = "Players", .items = NULL, .num_items = 0
};
static SimpleMenuItem *psel_items = NULL;
static int32_t psel_num_items = 0;
static int32_t psel_num_init_items = 0;
static char psel_player_name[20] = { [0] = '\0' };
static char psel_player_mac[sizeof("00:00:00:00:00:00")] = { [0] = '\0' };
static player_selection_done_callback_t psel_cb;
static void* psel_user_context;

static void free_items(void);


static void window_load(Window *window) {
	// Get the root layer
	Layer *window_layer = window_get_root_layer(window);

	// Get the bounds of the window for sizing the text layer
	GRect bounds = layer_get_bounds(window_layer);

	psel_sections.items = psel_items;
	psel_sections.num_items = psel_num_items;
	psel_menu_layer = simple_menu_layer_create(bounds, window, &psel_sections, 1, NULL);
	layer_add_child(window_layer, simple_menu_layer_get_layer(psel_menu_layer));
}

static void window_unload(Window *window) {
	simple_menu_layer_destroy(psel_menu_layer);
	free_items();
}

static void realloc_items(int32_t nb_to_alloc) {
	if (!psel_items || nb_to_alloc > psel_num_init_items) {
		psel_items = realloc(psel_items, nb_to_alloc * sizeof(SimpleMenuItem));
		psel_num_init_items = nb_to_alloc;
	}
}

static void item_select_callback(int index, void *context) {
	if (index >= psel_num_items)
		return;
	if (psel_items[index].title)
		strncpy(psel_player_name, psel_items[index].title, sizeof(psel_player_name));
	if (psel_items[index].subtitle)
		strncpy(psel_player_mac, psel_items[index].subtitle, sizeof(psel_player_mac));
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Sel Player: '%s' [%s]", psel_player_name, psel_player_mac);

	psel_cb(psel_user_context);
	window_stack_pop(true);
}

static void free_items(void) {
	for (int i = 0; i < psel_num_items; i++) {
		sfree((char*)psel_items[i].title);
		sfree((char*)psel_items[i].subtitle);
	}
	psel_num_init_items = 0;
	psel_num_items = 0;
	nfree(psel_items);
}

static void init_items(int32_t nb_to_alloc) {
	realloc_items(nb_to_alloc);
	psel_num_items = 0;
}

int32_t player_selection_add_item(void) {
	int32_t idx = psel_num_items;
	psel_num_items += 1;
	realloc_items(psel_num_items);
	psel_items[idx].title = NULL;
	psel_items[idx].subtitle = NULL;
	psel_items[idx].callback = item_select_callback;
	psel_items[idx].icon = NULL;
	return idx;
}

size_t player_selection_set_item_name(int32_t idx, const char *name) {
	if (idx >= psel_num_items)
		return -1;

	size_t len = strlen(name);
	char* c = malloc(len);
	strncpy(c, name, len);
	psel_items[idx].title = c;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "add name (idx %d, len %d): %s", (int)idx, len, c);
	return len;
}

size_t player_selection_set_item_mac(int32_t idx, const char *mac) {
	if (idx >= psel_num_items)
		return -1;

	size_t len = strlen(mac);
	char* c = malloc(len);
	strncpy(c, mac, len);
	psel_items[idx].subtitle = c;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "add mac (idx %d, len %d): %s", (int)idx, len, c);
	return len;
}

void player_selection_alloc_items(int32_t nb) {
	free_items();
	init_items(nb);
}

Window* player_selection_create_window(player_selection_done_callback_t callback, void* context) {
	Window* win = window_create();
	window_set_window_handlers(win,
		(WindowHandlers) {
			.load = window_load,
			.unload = window_unload,
		}
	);
	psel_cb = callback;
	psel_user_context = context;
	return win;
}

const char* player_selection_get_selected_player_mac(void) {
	return psel_player_mac;
}

const char* player_selection_get_selected_player_name(void) {
	return psel_player_name;
}

