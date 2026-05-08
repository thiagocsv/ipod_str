#include "audioTools.h"

void buildPlayer(audioClass *self) {
    self->isPlaying = false;
    self->isPaused = false;

    audio_pipeline_cfg_t pipeline = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audio_pipeline_init(&self->pipeline, &pipeline);

    fatfs_stream_cfg_t fatfs = FATFS_STREAM_CFG_DEFAULT();
    fatfs.type = AUDIO_STREAM_READER;
    self->sdreader = fatfs_stream_init(&fatfs);

    mp3_decoder_cfg_t mp3 = DEFAULT_MP3_DECODER_CONFIG();
    mp3.id3_parse_enable = true;
    self->mp3decoder = mp3_decoder_init(&mp3);

    i2s_stream_cfg_t i2s = I2S_STREAM_CFG_DEFAULT();
    i2s.type = AUDIO_STREAM_WRITER;
    self->i2sout = i2s_stream_init(&i2s);

    audio_pipeline_register(self->pipeline, self->sdreader, "file");
    audio_pipeline_register(self->pipeline, self->mp3decoder, "mp3");
    audio_pipeline_register(self->pipeline, self->i2sout, "i2s");

    const char *link_tag[3] = {"file", "mp3", "i2s"};
    audio_pipeline_link(self->pipeline, &link_tag[0], 3);

    audio_event_iface_cfg_t evt = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    self->evt = audio_event_iface_init(&evt);
    audio_pipeline_set_listener(self->pipeline, self->evt);
}

void playSound(audioClass *self, const char *datapath)
{
    if(self->isPlaying) stopSound(self);
    
    audio_element_set_uri(self->sdreader, datapath);
    audio_pipeline_reset_ringbuffer(self->pipeline);
    audio_pipeline_reset_elements(self->pipeline);
    audio_pipeline_change_state(self->pipeline, AEL_STATE_INIT);

    audio_pipeline_run(self->pipeline);
    self->isPlaying = true;
    self->isPaused = false;
}

void pauseSound(audioClass *self)
{
    if (self->isPlaying && !self->isPaused)
    {
        audio_pipeline_pause(self->pipeline);
        self->isPaused = true;
    }
}

void resumeSound(audioClass *self)
{
    if (self->isPlaying && self->isPaused)
    {
        audio_pipeline_resume(self->pipeline);
        self->isPaused = false;
    }
}

void stopSound(audioClass *self)
{
    if(!self->isPlaying)
        return;

    audio_pipeline_stop(self->pipeline);

    audio_pipeline_wait_for_stop(self->pipeline);

    audio_pipeline_terminate(self->pipeline);

    self->isPlaying = false;
    self->isPaused = false;
}

bool checkSongFinished(audioClass *self)
{
    if (!self->isPlaying) return false;
    audio_event_iface_msg_t msg;

    if (audio_event_iface_listen(self->evt, &msg, pdMS_TO_TICKS(10)) == ESP_OK)
    {
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) self->mp3decoder && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED)))
        {
            self->isPlaying = false;
            return true;
        }
    }
    return false;
}

void getMetadata(audioClass *self, char *title, char *artist)
{
    const esp_id3_info_t *id3_info = mp3_decoder_get_id3_info(self->mp3decoder);

    if(id3_info != NULL)
    {
        if(id3_info->title != NULL)
            strcpy(title, id3_info->title, 63);
            title[63] = '\0';
        else
            strcpy(title, "Unknown");

        if(id3_info->artist != NULL)
            strcpy(artist, id3_info->artist, 31);
        else
            strcpy(artist, "Unknown");
    }
    else
    {
        strcpy(title, "Without ID3");
        strcpy(artist, "");
    }
}
