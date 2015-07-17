#include "song_info.h"

#define STR_CMP_LEN      10

#define STR_ALBUM_LEN    20
#define STR_ARTIST_LEN   20
#define STR_TITEL_LEN    40
#define STR_CTITEL_LEN   40

//#define WINDOW_MARGIN_LEFT   5
//#define WINDOW_MARGIN_RIGHT  5
//#define WINDOW_MARGIN_TOP    5

#define TEXT_LAYER_ALBUM_HEIGTH   16
#define TEXT_LAYER_ARTIST_HEIGTH  16
#define TEXT_LAYER_TITEL_HEIGTH   20
#define TEXT_LAYER_CTITEL_HEIGTH  20

static char album[STR_ALBUM_LEN];
static char artist[STR_ARTIST_LEN];
static char titel[STR_TITEL_LEN];
static char ctitel[STR_CTITEL_LEN];

TextLayer* album_text_layer = NULL;
TextLayer* artist_text_layer = NULL;
TextLayer* titel_text_layer = NULL;
TextLayer* ctitel_text_layer = NULL;

void song_info_set_album(const char* s) {
	strncpy(album, s, STR_ALBUM_LEN);
}

void song_info_set_artist(const char* s) {
	strncpy(artist, s, STR_ARTIST_LEN);
}

void song_info_set_titel(const char* s) {
	strncpy(titel, s, STR_TITEL_LEN);
}

void song_info_set_ctitel(const char* s) {
	strncpy(ctitel, s, STR_CTITEL_LEN);
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

	ctitel_text_layer = text_layer_create(
		GRect(0, TEXT_LAYER_ALBUM_HEIGTH + TEXT_LAYER_ARTIST_HEIGTH + TEXT_LAYER_TITEL_HEIGTH, frame.size.w, TEXT_LAYER_CTITEL_HEIGTH));
	text_layer_set_text(ctitel_text_layer, ctitel);

	layer_add_child(layer, (Layer*)album_text_layer);
	layer_add_child(layer, (Layer*)artist_text_layer);
	layer_add_child(layer, (Layer*)titel_text_layer);
	layer_add_child(layer, (Layer*)ctitel_text_layer);

	return layer;
}