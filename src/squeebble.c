#include <pebble.h>

static Window* main_window;
static ActionBarLayer* action_bar;
static StatusBarLayer *status_bar;

// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1,
	COMMAND_KEY = 2,
};


// Write message to buffer & send
void send_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, 0x1);
	
	dict_write_end(iter);
	app_message_outbox_send();
}


// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {
}


// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}


static void button_up_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Button UP");
}

static void button_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Button SELECT");
}

static void button_down_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Button DOWN");
}


static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) button_down_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, button_select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, button_up_click_handler);
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
	// The loading of the icons is omitted for brevity... See gbitmap_create_with_resource()
//	action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, my_icon_previous, true);
//	action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, my_icon_next, true);
	// action_bar_layer_set_context(ActionBarLayer * action_bar, void * context)

	status_bar = status_bar_layer_create();
	// Change the status bar width to make space for the action bar
	int16_t width = layer_get_bounds(window_layer).size.w - ACTION_BAR_WIDTH;
	GRect frame = GRect(0, 0, width, STATUS_BAR_LAYER_HEIGHT);
	layer_set_frame(status_bar_layer_get_layer(status_bar), frame);
	layer_add_child(window_layer, status_bar_layer_get_layer(status_bar));

	status_bar_layer_set_colors(status_bar, GColorWhite, GColorBlack);
	//status_bar_layer_set_separator_mode(status_bar, StatusBarLayerSeparatorModeDotted);
}


static void main_window_unload(Window *window) {
	// Destroy Window's child Layers here
	action_bar_layer_destroy(action_bar);
}


void init(void) {
	main_window = window_create();
	window_set_window_handlers(main_window,
		(WindowHandlers) {
			.load = main_window_load,
			.unload = main_window_unload,
		}
	);
	window_stack_push(main_window, true);

	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

	send_message();
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}