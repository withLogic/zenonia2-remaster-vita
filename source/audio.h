#ifndef AUDIO_H
#define AUDIO_H

#include "utils/logger.h"

#ifdef __cplusplus
extern "C"
{
#endif

void audio_init();
void audio_cleanup();
void audio_play_sound(int sndID, int vol, int isLoop);
void audio_stop_sound();

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H