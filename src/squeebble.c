#include <pebble.h>
#include "player_selection.h"
#include "song_info.h"

static Window* main_window;
static ActionBarLayer* action_bar;
static StatusBarLayer *status_bar;
static GBitmap *bitmap_volume_up;
static GBitmap *bitmap_volume_down;
static GBitmap *bitmap_play;
static GBitmap *bitmap_pause;
static GBitmap *bitmap_next;
static GBitmap *bitmap_prev;

static Window* player_selection_window;


// Key values for AppMessage Dictionary
enum {
	STATUS_KEY     = 0,
	MESSAGE_KEY    = 1,
	COMMAND_KEY    = 2,
	// squeeze specific keys
	S_TRACK_KEY    = 10,
	S_MODE_KEY     = 11,
	S_VOLUME_KEY   = 12,
	S_DURATION_KEY = 13,
	S_TIME_KEY     = 14,
	S_ALBUM_KEY    = 15,
	S_ARTIST_KEY   = 16,
	S_TITLE_KEY    = 17,
	s_CTITEL_KEY   = 18,
};

enum squeeze_cmds {
	SC_STATUS = 1,
	SC_PLAY,
	SC_PAUSE,
	SC_NEXT,
	SC_PREV,
	SC_VOLUME,

	SC_PLAYERS = 10,     // [nb of entries as uint8_t] + nb*[Mac Addr + Name as String ("00:00:00:00:00:00:00", "Name")]
	SC_SEL_PLAYER,       // Mac Addr as String "00:00:00:00:00:00:00"
};

enum actionbar_mode {
	AB_MODE_VOLUME,
	AB_MODE_PLAY,
	AB_MODE_SKIP,
	AB_MODE_NUMBER_MODES,
	AB_MODE_DEFAULT = AB_MODE_VOLUME,
};
static enum actionbar_mode actionbar_mode;

void actionbar_set_mode(enum actionbar_mode mode) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "AB mode: %d", mode);
	switch (mode) {
		case AB_MODE_VOLUME:
			action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, bitmap_volume_up, true);
			action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, bitmap_volume_down, true);
			break;
		case AB_MODE_PLAY:
			action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, bitmap_play, true);
			action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, bitmap_pause, true);
			break;
		case AB_MODE_SKIP:
			action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, bitmap_prev, true);
			action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, bitmap_next, true);
			break;
		default:
			actionbar_set_mode(AB_MODE_DEFAULT);
			return;
	}
	actionbar_mode = mode;
}


// Write message to buffer & send
void send_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, 0x1);
	
	dict_write_end(iter);
	app_message_outbox_send();
}

void send_squeeze_cmd(enum squeeze_cmds cmd) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, COMMAND_KEY, cmd);
	dict_write_end(iter);
	app_message_outbox_send();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "cmd sent");
}

void send_squeeze_cmd_with_msg_string(enum squeeze_cmds cmd, const char* msg) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, COMMAND_KEY, cmd);
	dict_write_cstring(iter, MESSAGE_KEY, msg);
	dict_write_end(iter);
	app_message_outbox_send();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "cmd+string sent");
}



// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;

	uint32_t cmd = 0;
	uint32_t status = 0;

	// first check if there's a command set
	tuple = dict_find(received, COMMAND_KEY);
	if (tuple) {
		cmd = tuple->value->uint32;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Command: %lu", cmd);

		// a command needs a status
		tuple = dict_find(received, STATUS_KEY);
		if (tuple) {
			status = tuple->value->uint32;
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)status);
		}

		switch (cmd) {
			case SC_PLAYERS:
				tuple = dict_find(received, MESSAGE_KEY);
				if (tuple) {
					uint8_t nb = tuple->value->uint8;
					bool mac = true;
					uint32_t len = 1;
					int32_t item_idx = 0;
					player_selection_alloc_items(nb);
					while ((len < (tuple->length)) && (item_idx < nb)) {
						size_t l;
						if (mac) {
							item_idx = player_selection_add_item();
							l = player_selection_set_item_mac(item_idx, &tuple->value->cstring[len]);
						} else {
							l = player_selection_set_item_name(item_idx, &tuple->value->cstring[len]);
							//item_idx += 1;
						}
						mac = !mac;
						//APP_LOG(APP_LOG_LEVEL_DEBUG, "Cmd SC_PLAYERS: msg: %s", &tuple->value->cstring[len]);
						len += l + 1;
					}
					window_stack_push(player_selection_window, true);
				}
				break;
			case SC_SEL_PLAYER:
				// check status to see if player selection was successfull
				if (status != 0) {
					APP_LOG(APP_LOG_LEVEL_DEBUG, "failed to select player");
				}
				break;
			case SC_STATUS:
				tuple = dict_read_first(received);
				while (tuple) {
					switch (tuple->key) {
						case S_TRACK_KEY:
						case S_MODE_KEY:
						case S_VOLUME_KEY:
						case S_DURATION_KEY:
						case S_TIME_KEY:
						case S_ALBUM_KEY:
							song_info_set_album(tuple->value->cstring);
							break;
						case S_ARTIST_KEY:
							song_info_set_artist(tuple->value->cstring);
							break;
						case S_TITLE_KEY:
							song_info_set_titel(tuple->value->cstring);
							break;
						case s_CTITEL_KEY:
							song_info_set_ctitel(tuple->value->cstring);
							break;
						default:
							break;
					}
					tuple = dict_read_next(received);
				}
				break;
			default:
				break;
		}
	}
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "dropped");
}


// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "failed");
}


static void button_up_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Button UP");
	switch (actionbar_mode) {
		case AB_MODE_VOLUME:
			send_squeeze_cmd_with_msg_string(SC_VOLUME, "+5");
			break;
		case AB_MODE_PLAY:
			send_squeeze_cmd(SC_PLAY);
			break;
		case AB_MODE_SKIP:
			send_squeeze_cmd(SC_PREV);
			break;
		default:
			break;
	}
}

static void button_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Button SELECT");
	send_squeeze_cmd(SC_STATUS);
	actionbar_set_mode(actionbar_mode + 1);
}

static void button_down_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Button DOWN");
		switch (actionbar_mode) {
		case AB_MODE_VOLUME:
			send_squeeze_cmd_with_msg_string(SC_VOLUME, "-5");
			break;
		case AB_MODE_PLAY:
			send_squeeze_cmd(SC_PAUSE);
			break;
		case AB_MODE_SKIP:
			send_squeeze_cmd(SC_NEXT);
			break;
		default:
			break;
	}
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_DOWN, button_down_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, button_select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, button_up_click_handler);
}

static void player_selection_done_handler(void *context) {
	send_squeeze_cmd_with_msg_string(SC_SEL_PLAYER, player_selection_get_selected_player_mac());
}

static void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	// Create Window's child Layers here
	// Initialize the action bar:
	action_bar = action_bar_layer_create();
	// Associate the action bar with the window:
	action_bar_layer_add_to_window(action_bar, window);
	// Set the click config provider:
	action_bar_layer_set_click_config_provider(action_bar,
			click_config_provider);
	// Set the icons:
	//action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, bitmap_play, true);
	//action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, bitmap_pause, true);
	actionbar_set_mode(AB_MODE_DEFAULT);
	// action_bar_layer_set_context(ActionBarLayer * action_bar, void * context)

	status_bar = status_bar_layer_create();
	// Change the status bar width to make space for the action bar
	int16_t width = layer_get_bounds(window_layer).size.w - ACTION_BAR_WIDTH;
	GRect frame = GRect(0, 0, width, STATUS_BAR_LAYER_HEIGHT);
	layer_set_frame(status_bar_layer_get_layer(status_bar), frame);
	layer_add_child(window_layer, status_bar_layer_get_layer(status_bar));

	status_bar_layer_set_colors(status_bar, GColorWhite, GColorBlack);
	//status_bar_layer_set_separator_mode(status_bar, StatusBarLayerSeparatorModeDotted);

	player_selection_window = player_selection_create_window(player_selection_done_handler, NULL);

	GRect b = layer_get_bounds(window_layer);
	Layer *bla = song_info_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, width, b.size.h - STATUS_BAR_LAYER_HEIGHT));
	song_info_set_album("hans");
	layer_add_child(window_layer, bla);

	if (!bluetooth_connection_service_peek()) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "no bt service");
	}
}


static void main_window_unload(Window *window) {
	// Destroy Window's child Layers here
	action_bar_layer_destroy(action_bar);
}


void init(void) {
	bitmap_volume_down = gbitmap_create_with_resource(RESOURCE_ID_IMG_VOLUME_DOWN);
	bitmap_volume_up = gbitmap_create_with_resource(RESOURCE_ID_IMG_VOLUME_UP);
	bitmap_play = gbitmap_create_with_resource(RESOURCE_ID_IMG_PLAY);
	bitmap_pause = gbitmap_create_with_resource(RESOURCE_ID_IMG_PAUSE);
	bitmap_next = gbitmap_create_with_resource(RESOURCE_ID_IMG_NEXT);
	bitmap_prev = gbitmap_create_with_resource(RESOURCE_ID_IMG_PREV);

	main_window = window_create();
	window_set_window_handlers(main_window,
		(WindowHandlers) {
			.load = main_window_load,
			.unload = main_window_unload,
			.appear = NULL,
			.disappear = NULL,
		}
	);
	window_stack_push(main_window, true);

	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	//send_squeeze_cmd(SC_STATUS);
}

void deinit(void) {
	gbitmap_destroy(bitmap_volume_down);
	gbitmap_destroy(bitmap_volume_up);
	gbitmap_destroy(bitmap_play);
	gbitmap_destroy(bitmap_pause);
	gbitmap_destroy(bitmap_prev);
	gbitmap_destroy(bitmap_next);

	app_message_deregister_callbacks();
	window_destroy(main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}