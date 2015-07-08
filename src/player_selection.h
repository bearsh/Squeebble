#include <pebble.h>

typedef void (*player_selection_done_callback_t)(void *context);

int32_t player_selection_add_item(void);
size_t player_selection_set_item_name(int32_t idx, const char *name);
size_t player_selection_set_item_mac(int32_t idx, const char *mac);
void player_selection_alloc_items(int32_t nb);
Window* player_selection_create_window(player_selection_done_callback_t callback, void* context);
const char* player_selection_get_selected_player_mac(void);
const char* player_selection_get_selected_player_name(void);
