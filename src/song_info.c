#include "song_info.h"

#define STR_CMP_LEN      10

#define STR_ALBUM_LEN    20
#define STR_ARTIST_LEN   20
#define STR_TITEL_LEN    40
#define STR_CTITEL_LEN   40

#define TEXT_LAYER_ALBUM_HEIGTH   16
#define TEXT_LAYER_ARTIST_HEIGTH  16
#define TEXT_LAYER_TITEL_HEIGTH   50
#define TEXT_LAYER_CTITEL_HEIGTH  20

#define PROGESSBAR_LAyER_POS_Y    120
#define PROGESSBAR_LAyER_HEIGHT   10

#define TIME_1S     1000 /*ms*/

static char album[STR_ALBUM_LEN];
static char artist[STR_ARTIST_LEN];
static char titel[STR_TITEL_LEN];
static char ctitel[STR_CTITEL_LEN];

TextLayer* album_text_layer = NULL;
TextLayer* artist_text_layer = NULL;
TextLayer* titel_text_layer = NULL;
TextLayer* ctitel_text_layer = NULL;
Layer* progressbar_layer = NULL;

GPath* progressbar_outline_path = NULL;
GPath* progressbar_fill_path = NULL;

static GPathInfo progressbar_outline_info = {
  .num_points = 4,
  .points = (GPoint []) {{5, 1}, {80, 1}, {80, 5}, {5, 5}}
};
static GPathInfo progressbar_fill_info = {
  .num_points = 4,
  .points = (GPoint []) {{5, 1}, {40, 1}, {40, 5}, {5, 5}}
};

static int32_t track_time = -1;
static int32_t track_duration = -1;
static AppTimer* duration_timer = NULL;

static inline void progressbar_set_x(int16_t x) {
	progressbar_fill_info.points[1].x = x + 5;
	progressbar_fill_info.points[2].x = x + 5;
}
static inline int16_t progressbar_width(void) {
	return progressbar_outline_info.points[1].x - 5;
}

static void update_time(void *data) {
	if (track_duration > 0 && track_time >= 0) {
		track_time++;
		layer_mark_dirty(progressbar_layer);
		duration_timer = app_timer_register(TIME_1S, update_time, NULL);
	}
}

static void progressbar_layer_update_proc(struct Layer *layer, GContext *ctx) {
	if (track_duration > 0) {
		int16_t x = 0;
		if (track_time > 0) {
			x = (track_time * progressbar_width() + track_duration/2) / track_duration;
			if (x > progressbar_width()) { // sanity check
				x = progressbar_width();
			}
		}
		progressbar_set_x(x);
		graphics_context_set_fill_color(ctx, GColorLightGray);
		gpath_draw_filled(ctx, progressbar_outline_path);
		graphics_context_set_fill_color(ctx, GColorBlack);
		gpath_draw_filled(ctx, progressbar_fill_path);
	}
}


void song_info_set_album(const char* s) {
	strncpy(album, s, STR_ALBUM_LEN);
}

void song_info_set_artist(const char* s) {
	strncpy(artist, s, STR_ARTIST_LEN);

	layer_mark_dirty(progressbar_layer);
}

void song_info_set_titel(const char* s) {
	strncpy(titel, s, STR_TITEL_LEN);
}

void song_info_set_ctitel(const char* s) {
	strncpy(ctitel, s, STR_CTITEL_LEN);
}


void song_info_update_time_duration(int32_t time, int32_t duration) {
	if (duration > 0 && time >= 0) {
		if (!duration_timer) {
			duration_timer = app_timer_register(TIME_1S, update_time, NULL);
		} else {
			app_timer_reschedule(duration_timer, TIME_1S);
		}
		track_duration = duration;
		track_time = time;
		layer_set_hidden(progressbar_layer, false);
	} else {
		if (duration_timer) {
			app_timer_cancel(duration_timer);
			duration_timer = NULL;
		}
		track_duration = -1;
		layer_set_hidden(progressbar_layer, true);
	}
	layer_mark_dirty(progressbar_layer);
}


Layer* song_info_layer_create(GRect frame) {
	Layer* layer = layer_create	(frame);

	album[0] = '\0';
	artist[0] = '\0';
	titel[0] = '\0';
	ctitel[0] = '\0';

	album_text_layer =  text_layer_create(
		GRect(0, 0, frame.size.w, TEXT_LAYER_ALBUM_HEIGTH));
	text_layer_set_text(album_text_layer, album);

	artist_text_layer = text_layer_create(
		GRect(0, TEXT_LAYER_ALBUM_HEIGTH, frame.size.w, TEXT_LAYER_ARTIST_HEIGTH));
	text_layer_set_text(artist_text_layer, artist);

	titel_text_layer = text_layer_create(
		GRect(0, TEXT_LAYER_ALBUM_HEIGTH + TEXT_LAYER_ARTIST_HEIGTH, frame.size.w, TEXT_LAYER_TITEL_HEIGTH));
	text_layer_set_text(titel_text_layer, titel);
	text_layer_set_font(titel_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	ctitel_text_layer = text_layer_create(
		GRect(0, TEXT_LAYER_ALBUM_HEIGTH + TEXT_LAYER_ARTIST_HEIGTH + TEXT_LAYER_TITEL_HEIGTH, frame.size.w, TEXT_LAYER_CTITEL_HEIGTH));
	text_layer_set_text(ctitel_text_layer, ctitel);

	progressbar_layer = layer_create(
		GRect(0, PROGESSBAR_LAyER_POS_Y, frame.size.w, PROGESSBAR_LAyER_HEIGHT));
	layer_set_update_proc(progressbar_layer, progressbar_layer_update_proc);
	progressbar_outline_info.points[1].x = frame.size.w - 5 - 1;
	progressbar_outline_info.points[2].x = frame.size.w - 5 - 1;
	progressbar_outline_path = gpath_create(&progressbar_outline_info);
	progressbar_fill_path = gpath_create(&progressbar_fill_info);
	progressbar_set_x(0);

	layer_add_child(layer, (Layer*)album_text_layer);
	layer_add_child(layer, (Layer*)artist_text_layer);
	layer_add_child(layer, (Layer*)titel_text_layer);
	layer_add_child(layer, (Layer*)ctitel_text_layer);
	layer_add_child(layer, progressbar_layer);

	return layer;
}