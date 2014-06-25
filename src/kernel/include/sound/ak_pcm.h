#ifndef _AK_PCM_H
#define _AK_PCM_H

#include <sound/core.h>
#include <sound/pcm.h>


struct ak_codec_dai;

struct ak_codec_ops {
	void (*dac_init) (struct ak_codec_dai *dai);
	void (*dac_exit) (struct ak_codec_dai *dai);

	void (*adc_init) (struct ak_codec_dai *dai);
	void (*adc_exit) (struct ak_codec_dai *dai);

	void (*set_dac_samplerate)(struct ak_codec_dai *dai, unsigned int rate);
	unsigned long (*set_adc_samplerate)(struct ak_codec_dai *dai, unsigned int rate);
	
	void (*set_dac_channels)(struct ak_codec_dai *dai, unsigned int channels);
	void (*set_adc_channels)(struct ak_codec_dai *dai, unsigned int channels);

	void (*playback_start)(struct ak_codec_dai *dai);
	void (*playback_end)(struct ak_codec_dai *dai);

	void (*capture_start)(struct ak_codec_dai *dai);
	void (*capture_end)(struct ak_codec_dai *dai);

	void (*start_to_play) (struct ak_codec_dai *dai, unsigned int channels, unsigned int rate);
};

struct ak_proc_entry {
	const char *name;
	void (*cb)(struct snd_info_entry *, struct snd_info_buffer *);
};

struct ak_codec_dai {
	int num_kcontrols;
	struct snd_kcontrol_new *kcontrols;
	int num_pentries;
	void *entries_private;
	struct ak_proc_entry *pentries;
	struct ak_codec_ops *ops;
	int  aec_flag;

};

int ak_codec_register(struct ak_codec_dai *dai);
void ak_codec_ctl_event(unsigned int iface, unsigned int event, const char* ctl_name);

#endif
