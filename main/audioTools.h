#ifndef AUDIOTOOLS_H
#define AUDIOTOOLS_H

#include "audio_pipeline.h"
#include "audio_element.h"
#include "audio_event_iface.h"
#include "fatfs_stream.h"
#include "mp3_decoder.h"
#include "i2s_stream.h"
#include <stdbool.h>
#include <string.h>

typedef struct {
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t sdreader;
    audio_element_handle_t mp3decoder;
    audio_element_handle_t i2sout;
    audio_event_iface_handle_t evt;
    bool isPlaying;
    bool isPaused;
} audioClass;

void buildPlayer(audioClass *self);

void playSound(audioClass *self, const char *datapath);

void pauseSound(audioClass *self);

void resumeSound(audioClass *self);

void stopSound(audioClass *self);

bool checkSongFinished(audioClass *self);

void getMetadata(audioClass *self, char *title, char *artist);

#endif
