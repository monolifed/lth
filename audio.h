#ifndef LTH_AUDIO_H
#define LTH_AUDIO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Audio_Decoder Audio_Decoder;

typedef struct Audio_Buffer Audio_Buffer;

typedef struct Audio_Source Audio_Source;

void aud_load(void);
Audio_Source *aud_make_source_from_buffer(Audio_Buffer *buf);
Audio_Source *aud_make_source(const void *data, size_t size);
void aud_release_buffer(Audio_Buffer *buffer);
void aud_play(Audio_Source *source);
void aud_set_source_looped(Audio_Source *src, bool looped);
void aud_delete_source(Audio_Source *source);
Audio_Buffer *aud_make_buffer(const void *data, int size);
bool aud_is_playing(Audio_Source *src);
void aud_set_volume(Audio_Source *src, float volume);

#endif // LTH_AUDIO_H
