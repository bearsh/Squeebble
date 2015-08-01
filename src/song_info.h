#include <pebble.h>

void song_info_set_album(const char* s);
void song_info_set_artist(const char* s);
void song_info_set_titel(const char* s);
void song_info_set_ctitel(const char* s);
void song_info_update_time_duration(int32_t time, int32_t duration);
Layer* song_info_layer_create(GRect frame);