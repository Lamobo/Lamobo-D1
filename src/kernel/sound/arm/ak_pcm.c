/*
 *  akpcm soundcard
 *  Copyright (c) by Anyka, Inc.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/export.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/completion.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <asm/bitops.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/tlv.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/info.h>
#include <sound/initval.h>
#include <linux/cpufreq.h>
//#include <linux/anyka_cpufreq.h>

#include <plat/l2.h>
#include <sound/ak_pcm.h>
#include <mach/ak_codec.h>
#include <linux/vmalloc.h>
#include <linux/reboot.h>
#include <mach-anyka/aec_interface.h>

//#define CONFIG_PCM_DUMP              1
#define AK_PCM_DELAY_CLOSE_DAC

unsigned long playback_statu;

struct snd_akpcm {
	struct snd_card *card;
	struct snd_pcm *pcm;
	spinlock_t mixer_lock;
	struct snd_pcm_substream *playbacksubstrm;
	struct snd_pcm_substream *capturesubstrm;
	struct completion playbackHWDMA_completion;
	struct completion captureHWDMA_completion;
	u8   L2BufID_For_DAC;
	u8   L2BufID_For_ADC23;
	snd_pcm_uframes_t PlaybackCurrPos;
	snd_pcm_uframes_t CaptureCurrPos;
	unsigned long   playbackStrmDMARunning;//bit[0]:strm state(running or not). bit[1]:DMA state(running or finished). bit[2]:strm state(suspend in frequest changed).
	unsigned long   captureStrmDMARunning;//bit[0]:strm state(running or not). bit[1]:DMA state(running or finished)

	struct tasklet_struct	fetch_tasklet;
	void *	pp_buf_addr[2];	//Pingpong Buffer address
	int		playing_idx;
	snd_pcm_uframes_t optimal_period_size;	
	int 		optimal_period_bytes;
	bool		use_optimal_period;

	struct timer_list stopoutput_work_timer;
	struct tasklet_struct	close_dac;

	struct ak_codec_dai *dai;
	struct ak_codec_ops *ops;

#ifdef CONFIG_CPU_FREQ
	struct notifier_block	freq_transition;
#endif

//#ifdef CONFIG_SUPPORT_AEC
	struct tasklet_struct capture_aec_tasklet;
	struct tasklet_struct playback_aec_tasklet;
	T_AEC_INPUT p_aecin;
	T_AEC_BUF	p_aecbufs;
	T_VOID		*pfilter;
	unsigned char *playback_data;
	unsigned char *capture_data;
	unsigned char *temp;
//#endif

#ifdef CONFIG_PCM_DUMP
	void*				pcmDumpDataBuffer;
	bool				enableDump;
	unsigned long		pcmDumpSize;
	struct   timeval    pcmDumpTime;
#endif

	struct delayed_work ds_work; //delay start work
};

struct captureSync{
	unsigned long long adcCapture_bytes;
	struct timeval tv;
	unsigned int rate;
	unsigned int frame_bits;
};

struct captureSync capSync;

static unsigned long long dac_clock;


/*************
 * PCM interface
 *************/
#define USE_FORMATS 		(SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S16_LE)
#define USE_RATE			(SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_96000)

#define CAPTURE_USE_RATE_MIN		5500
#define CAPTURE_USE_RATE_MAX		96000

#define PLAYBACK_USE_RATE_MIN		5500
#define PLAYBACK_USE_RATE_MAX		192000

#define CAPTURE_USE_CHANNELS_MIN 	1
#define CAPTURE_USE_CHANNELS_MAX 	1

#define PLAYBACK_USE_CHANNELS_MIN 	2
#define PLAYBACK_USE_CHANNELS_MAX 	2

#define akpcm_playback_buf_bytes_max     (64*1024)
#define akpcm_playback_period_bytes_min  512

#ifdef CONFIG_SUPPORT_AEC
#define akpcm_playback_period_bytes_max  512
#else
#define akpcm_playback_period_bytes_max  32768
#endif

#define akpcm_playback_period_aligned		128
#define akpcm_playback_periods_min       4

#define akpcm_playback_periods_max       2048

#define DELAY_TIME_FOR_CLOSING_DAC		(HZ * 30)


#define akpcm_capture_buf_bytes_max      (64*1024)
#define akpcm_capture_period_bytes_min   512
#define akpcm_capture_periods_min        4
#define akpcm_capture_periods_max        64

#ifdef CONFIG_PCM_DUMP
#define akpcm_dump_data_size           (2*1024*1024)
#endif


//#ifdef CONFIG_SUPPORT_AEC
#define		AEC_NN			128
#define		AEC_TAIL		(AEC_NN*10)
#define		AEC_CHANNELS	1
#define		AEC_SAMPLERATE 	8000
#define		AEC_BITSPERSAMPLE 16
//#endif


 static DEFINE_MUTEX(reboot_lock);
 static struct snd_akpcm *reboot_info = NULL;

 /**
 * @brief   tell camera driver the audio capture samples for AV sync 
 * @author  
 * @date   
 * @input   void
 * @output  void *
 * @return  void
 */
void *getRecordSyncSamples(void)
{
	return &capSync;
}
EXPORT_SYMBOL(getRecordSyncSamples);

/* return the dac_clock */
unsigned long long getPlaybackEclapseTime(void)
{
	return dac_clock;
}
EXPORT_SYMBOL(getPlaybackEclapseTime);

void ak_close_dac_timer(unsigned long data)
{
	struct snd_akpcm *akpcm = (struct snd_akpcm *)data;

	if (akpcm->ops->playback_end) {
		akpcm->ops->playback_end(akpcm->dai);
	}
}

//#ifdef CONFIG_SUPPORT_AEC
void  ak37pcm_playback_aec(unsigned long data)
{
	struct snd_akpcm *ak37pcm = (struct snd_akpcm *)data;
	struct snd_pcm_substream *substream = ak37pcm->playbacksubstrm;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_pcm_substream *capture_substream = ak37pcm->capturesubstrm;
	struct snd_pcm_runtime *capture_runtime;
	
	unsigned int	playback_pos;
	unsigned long period_bytes = frames_to_bytes(runtime,runtime->period_size);
	unsigned long buffer_bytes = frames_to_bytes(runtime,runtime->buffer_size);
	

	//Check if the capture substream is opened yet
	if (!capture_substream)
		return;

	//FIXME: What is the relationship between substream and its belonging runtime
	capture_runtime = capture_substream->runtime;
	if (!capture_runtime)
		return;

	//Check if the AEC library has been successfully opened.
	if (runtime->rate != AEC_SAMPLERATE || period_bytes != akpcm_playback_period_bytes_min || !ak37pcm->pfilter)
		return;

	//Make sure that the DMA of capturing is running
	if (test_bit(0,&ak37pcm->captureStrmDMARunning) && 
			test_bit(1,&ak37pcm->captureStrmDMARunning)) {

		//FIXME: Figure out the position of last period that has just been finished		
		if (ak37pcm->PlaybackCurrPos <= 0) {
			playback_pos = buffer_bytes	- period_bytes;
		} else {
			playback_pos = ak37pcm->PlaybackCurrPos - period_bytes;
		}

		//Something must go wrong.
		if (playback_pos % akpcm_playback_period_bytes_min != 0 || playback_pos < 0)
			printk("ak37pcm_playback_aec==>playback_pos=%d\n", playback_pos);

		/*
		  * FIXME: The AEC Lib just supports single channel by now. But we make a trick here by
		  * cheating the AEC Lib. Tha input data for the AEC Lib is actully stereo. By doing that, the
		  * system load could be lowered a little bit.
		  */
#if 0
		short *to = ak37pcm->playback_data;
		short *from = vaddr + playback_pos;
		int count = period_bytes / channels / 2;
		if (count != AEC_NN) 
			printk("ak37pcm_playback_aec==>count=%d\n", count);
		for (i = 0; i < count; i++) {
			to[i] = from[i * 2];
		}
#endif
		/*We assume the channels is stereo basing on the truth */
		#if 0
		ak37pcm->p_aecbufs.buf_near = ak37pcm->p_aecbufs.buf_out = NULL;
		ak37pcm->p_aecbufs.len_near = ak37pcm->p_aecbufs.len_out = 0;
	       ak37pcm->p_aecbufs.buf_far = vaddr + playback_pos;
		ak37pcm->p_aecbufs.len_far = period_bytes / channels;
		ret = AECLib_Control(ak37pcm->pfilter, &ak37pcm->p_aecbufs);
		if (ret < 0)
			printk("p=%d\n", ret);
		#endif
	}	
}

void ak37pcm_capture_aec(unsigned long data)
{
	struct snd_akpcm *ak37pcm = (struct snd_akpcm *)data;
	struct snd_pcm_substream *substream = ak37pcm->capturesubstrm;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_pcm_substream *playback_substream = ak37pcm->playbacksubstrm;
	struct snd_pcm_runtime *playback_runtime;

	//dma_addr_t vaddr = runtime->dma_area;
	dma_addr_t paddr = runtime->dma_addr;
	int period_bytes =  frames_to_bytes(runtime,runtime->period_size);
	int buffer_bytes = frames_to_bytes(runtime,runtime->buffer_size);
	int capture_pos;
	int ret;

	//Check if the playback substream is open
	if (!playback_substream)
		return;

	//FIXME: What is the relationship between substream and its belonging runtime
	playback_runtime = playback_substream->runtime;
	if (!playback_runtime)
		return;

	/*
	 *Check if the AEC library has been successfully opened.
	 */
	if (runtime->rate != AEC_SAMPLERATE || period_bytes != akpcm_capture_period_bytes_min || !ak37pcm->pfilter)
		return;

//	if (test_bit(0,&ak37pcm->playbackStrmDMARunning) && 
//		test_bit(1,&ak37pcm->playbackStrmDMARunning)) 
	{

		if (ak37pcm->CaptureCurrPos == 0) {
			capture_pos = buffer_bytes  - period_bytes;
		} else {
			capture_pos = ak37pcm->CaptureCurrPos - period_bytes;
		}
#if 0
		ak37pcm->p_aecbufs.buf_near = vaddr + capture_pos;
		ak37pcm->p_aecbufs.len_near =  period_bytes;
        	ak37pcm->p_aecbufs.buf_out = ak37pcm->p_aecbufs.buf_near;
		ak37pcm->p_aecbufs.len_out = ak37pcm->p_aecbufs.len_near;
       	ak37pcm->p_aecbufs.buf_far = NULL;
		ak37pcm->p_aecbufs.len_far = 0;
		if (ak37pcm->p_aecbufs.len_near != akpcm_capture_period_bytes_min)
				printk("ak37pcm_capture_aec==>len_near=%d\n", period_bytes);

		ret = AECLib_Control(ak37pcm->pfilter, &ak37pcm->p_aecbufs);
		if (ret < 0)
			printk("c=%d\n", ret);
#endif
		ak37pcm->p_aecbufs.buf_out = phys_to_virt(paddr+ak37pcm->CaptureCurrPos);
		ak37pcm->p_aecbufs.len_out = akpcm_capture_period_bytes_min;
	    ret = AECLib_Control(ak37pcm->pfilter, &ak37pcm->p_aecbufs);
	}
}
//#endif

/**
 * @brief  create new mixer interface
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static struct snd_card *cur_card;
static struct snd_pcm *cur_pcm;
static struct ak_codec_dai *cur_codec;
int ak_codec_register(struct ak_codec_dai *dai)
{
	int i, err;
	struct snd_akpcm *akpcm;
	struct snd_info_entry *entry;

	if (!dai)
		return -1;

	if (!dai->ops)
		return -1;

	if (!cur_card || !cur_pcm) {
		printk("AK PCM: No card, No cur_pcm(failed)\n");
		return -1;
	}

	akpcm = cur_card->private_data;

	if (cur_codec) {
		printk("AK PCM: some codec has registered first(failed)\n");
		return -1;
	} else {
		cur_codec = dai;
		akpcm->dai = dai;
		akpcm->ops = dai->ops;
	}

	strcpy(cur_card->mixername, "akpcm Mixer");

	for (i = 0; i < dai->num_kcontrols; i++) {
		err = snd_ctl_add(cur_card, snd_ctl_new1(&dai->kcontrols[i], dai));
		if (err < 0)
			return err;
	}
	printk("AK PCM: create num_kcontrols=%d\n", dai->num_kcontrols);

	// register a proc file to tell user whether the chip DAC module has been fixed
	for (i = 0; i < dai->num_pentries; i++) {
		snd_card_proc_new (cur_card, dai->pentries[i].name, &entry);
		snd_info_set_text_ops(entry, dai->entries_private, dai->pentries[i].cb);
	}
	
	return 0;
}

void ak_codec_ctl_event(unsigned int iface, unsigned int event, const char* ctl_name)
{
	struct snd_ctl_elem_id elem_id;
	struct snd_kcontrol *ctl_switch;

	/* find the corresponding switch control */	
	memset(&elem_id, 0, sizeof(elem_id));
	elem_id.iface = iface;

	strcpy(elem_id.name, ctl_name);

	ctl_switch = snd_ctl_find_id(cur_card, &elem_id);

	if (ctl_switch)
	    snd_ctl_notify(cur_card, event, &ctl_switch->id);
}

void alsabuf_to_ppbuf(unsigned long data)
{
	struct snd_akpcm *akpcm = (struct snd_akpcm *)data;
	struct snd_pcm_substream *substream = akpcm->playbacksubstrm;
	struct snd_pcm_runtime *runtime = substream->runtime;
	void *vaddr = runtime->dma_area;	
	snd_pcm_uframes_t avail;	
	int bytespersample = frames_to_bytes(runtime, 1);
	int buffer_bytes = frames_to_bytes(runtime,runtime->buffer_size);
	int period_bytes = akpcm->optimal_period_bytes;
	int pend_bytes;
	int dst_pos, cur_pos;
	void *dst_addr = NULL;

	if (akpcm->playing_idx == 1)
		dst_addr = akpcm->pp_buf_addr[0];
	else
		dst_addr = akpcm->pp_buf_addr[1];
	
	akpcm->PlaybackCurrPos += period_bytes;
	if(akpcm->PlaybackCurrPos >= buffer_bytes)
	{
		akpcm->PlaybackCurrPos = akpcm->PlaybackCurrPos - buffer_bytes;
	}

	avail = snd_pcm_playback_avail(runtime) + akpcm->optimal_period_size;
	pend_bytes = (runtime->buffer_size - avail) * bytespersample;

	if (avail >= runtime->stop_threshold) {
		printk("optimal=%u, original=%u, left=%u\n", (unsigned int)akpcm->optimal_period_size, 
			(unsigned int)runtime->period_size, (unsigned int)(runtime->buffer_size - snd_pcm_playback_avail(runtime)));
		return;
	}

	dst_pos = akpcm->PlaybackCurrPos + period_bytes;
	cur_pos = akpcm->PlaybackCurrPos;
	if (dst_pos > buffer_bytes) {
		memcpy(dst_addr, vaddr + cur_pos, (buffer_bytes - cur_pos));
		memcpy(dst_addr + (buffer_bytes - cur_pos), vaddr, dst_pos - buffer_bytes);
	} else {
		memcpy(dst_addr, vaddr + cur_pos, period_bytes);
	}

	if (pend_bytes < period_bytes)
		memset(dst_addr + pend_bytes, 0, period_bytes - pend_bytes);
}


/* from the previous interrupt hanlder */
void dac_exit_tasklet(unsigned long data)
{
	struct snd_akpcm *akpcm = (struct snd_akpcm *)data;

	if (akpcm->ops->dac_exit)
		akpcm->ops->dac_exit(akpcm->dai);
}

void akpcm_playback_interrupt_optimize(unsigned long data)
{
	struct snd_akpcm *akpcm = (struct snd_akpcm *)data;
	struct snd_pcm_substream *substream = akpcm->playbacksubstrm;
	struct snd_pcm_runtime *runtime = substream->runtime;
	u8 id = akpcm->L2BufID_For_DAC;
	int bytespersample = frames_to_bytes(runtime, 1);
	void *pdst_addr;

	snd_pcm_period_elapsed(substream);

	dac_clock += akpcm->optimal_period_bytes / bytespersample  * 1000000 / runtime->rate;

	if(test_bit(0,&playback_statu) 
		&& snd_pcm_playback_avail(runtime) < runtime->stop_threshold)//output stream is running
	{
		if (akpcm->playing_idx == 0)
			akpcm->playing_idx = 1;
		else 
			akpcm->playing_idx = 0;
		
		pdst_addr = (void *)virt_to_phys(akpcm->pp_buf_addr[akpcm->playing_idx]);		
		l2_combuf_dma((unsigned long)pdst_addr, id, akpcm->optimal_period_bytes, 
			(l2_dma_transfer_direction_t)MEM2BUF,1);

		tasklet_schedule(&akpcm->fetch_tasklet);
	}
	else //output strm has been stopped
	{
		printk("output stream stopped\n");
		clear_bit(1,&akpcm->playbackStrmDMARunning); //DMA has finished
		complete(&(akpcm->playbackHWDMA_completion));

#if defined(AK_PCM_DELAY_CLOSE_DAC)		
		/* delay to close output channel */
		akpcm->stopoutput_work_timer.expires = jiffies + DELAY_TIME_FOR_CLOSING_DAC;
		add_timer(&akpcm->stopoutput_work_timer);

		//close the dac in order to eliminate the noise caused by interrupt of 
		// DAC data transfer	
		tasklet_schedule(&akpcm->close_dac);
#endif		
	}
}


/**
 * @brief  DMA transfer for playback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
void akpcm_playback_interrupt(unsigned long data)
{
	struct snd_akpcm *akpcm = (struct snd_akpcm *)data;
	struct snd_pcm_substream *substream = akpcm->playbacksubstrm;
	struct snd_pcm_runtime *runtime = substream->runtime;
	dma_addr_t paddr = runtime->dma_addr;
	void *vaddr = runtime->dma_area;	

	u8 id = akpcm->L2BufID_For_DAC;
	unsigned long period_bytes = 0;
	unsigned long buffer_bytes = 0;
	int bytespersample = frames_to_bytes(runtime, 1);
	snd_pcm_uframes_t avail;
	unsigned long pend_bytes;

	period_bytes = frames_to_bytes(runtime,runtime->period_size);
	buffer_bytes = frames_to_bytes(runtime,runtime->buffer_size);
	akpcm->PlaybackCurrPos += period_bytes;
	if(akpcm->PlaybackCurrPos >= buffer_bytes)
	{
		akpcm->PlaybackCurrPos = 0;
	}
	snd_pcm_period_elapsed(substream);

	dac_clock += period_bytes / bytespersample  * 1000000 / runtime->rate;
	avail = snd_pcm_playback_avail(runtime);
	pend_bytes = (runtime->buffer_size - avail) * bytespersample;
#if 0
	int i;
	unsigned short s;

	printk("_________________\n");
	for (i=0; i<64; i++)
	{
		s = *(unsigned short *)__va(paddr+akpcm->CaptureCurrPos+i*2);
		if (s < 0x8000)
			printk("%d\n", s);
		else
			printk("-%d\n", (unsigned short)(~s+1));
	}
	printk("_________________\n");
#endif


#ifdef CONFING_SUPPORT_AEC
//	tasklet_schedule(&akpcm->playback_aec_tasklet);
#endif
	
	if(test_bit(0,&playback_statu) && avail < runtime->stop_threshold)//output stream is running
	{
		//printk(KERN_ERR "begin next dma");
		if (pend_bytes < period_bytes)
			memset(vaddr+akpcm->PlaybackCurrPos + pend_bytes, 0, period_bytes - pend_bytes);

		//#ifdef CONFIG_SUPPORT_AEC
		if( akpcm->dai->aec_flag == 1)
		{
			AECLib_DacInt(akpcm->pfilter, phys_to_virt(paddr+akpcm->PlaybackCurrPos), period_bytes);
		}
		//memcpy(akpcm->playback_data, akpcm->temp, period_bytes);
		//#endif

		l2_combuf_dma(paddr+akpcm->PlaybackCurrPos, id, period_bytes, 
			(l2_dma_transfer_direction_t)MEM2BUF,1);
		
#ifdef CONFIG_PCM_DUMP
		if(akpcm->enableDump){
			unsigned char *pcmDump_dma_area=runtime->dma_area;
			if((akpcm->pcmDumpSize+akpcm_playback_period_bytes_min)<=akpcm_dump_data_size){
				//printk(KERN_ERR "akpcm->dumpsize=%d,akpcm->PlaybackCurrPos=%d \n",akpcm->dumpsize,akpcm->PlaybackCurrPos);
				memcpy(akpcm->pcmDumpDataBuffer+akpcm->pcmDumpSize,pcmDump_dma_area+akpcm->PlaybackCurrPos,akpcm_playback_period_bytes_min);
				akpcm->pcmDumpSize+=akpcm_playback_period_bytes_min;
			}else{
				printk(KERN_ERR "akpcm->enableDump false\n");
				akpcm->enableDump=false;
			}
		}
#endif
		
	}
	else //output strm has been stopped
	{
		printk("output stream stopped\n");
		clear_bit(1,&akpcm->playbackStrmDMARunning); //DMA has finished
		complete(&(akpcm->playbackHWDMA_completion));

#if defined(AK_PCM_DELAY_CLOSE_DAC)		
		/* delay to close output channel */
		akpcm->stopoutput_work_timer.expires = jiffies + DELAY_TIME_FOR_CLOSING_DAC;
		add_timer(&akpcm->stopoutput_work_timer);

		//close the dac in order to eliminate the noise caused by interrupt of 
		// DAC data transfer	
		tasklet_schedule(&akpcm->close_dac);
#endif
	}
}

/**
 * @brief  DMA transfer for capture
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
void akpcm_capture_interrupt(unsigned long data)
{
	struct snd_akpcm *akpcm = (struct snd_akpcm *)data;
	struct snd_pcm_substream *substream = akpcm->capturesubstrm;
	struct snd_pcm_runtime *runtime = substream->runtime;
	dma_addr_t paddr = runtime->dma_addr;
	u8 id = akpcm->L2BufID_For_ADC23;
	
	unsigned long period_bytes = 0;
	unsigned long buffer_bytes = 0;
	period_bytes = frames_to_bytes(runtime,runtime->period_size);
	buffer_bytes = frames_to_bytes(runtime,runtime->buffer_size);
	do_gettimeofday(&capSync.tv);
	capSync.adcCapture_bytes += period_bytes;
	akpcm->CaptureCurrPos += period_bytes;
	if(akpcm->CaptureCurrPos >= buffer_bytes)
	{
		akpcm->CaptureCurrPos = 0;
	}
	snd_pcm_period_elapsed(substream);

//#ifdef CONFIG_SUPPORT_AEC
	if( akpcm->dai->aec_flag == 1)
	{
		tasklet_schedule(&akpcm->capture_aec_tasklet);
	}
//#endif
	if(test_bit(0,&akpcm->captureStrmDMARunning)) //input stream is running
	{
		
		//#ifdef CONFIG_SUPPORT_AEC
		if( akpcm->dai->aec_flag == 1)
		{
			AECLib_AdcInt(akpcm->pfilter, akpcm->capture_data, akpcm_capture_period_bytes_min); 
		
	
			l2_combuf_dma(virt_to_phys(akpcm->capture_data), id, akpcm_capture_period_bytes_min, 
			(l2_dma_transfer_direction_t)BUF2MEM,1);
		}
		//#else
		else
		{
			l2_combuf_dma(paddr+akpcm->CaptureCurrPos, id, akpcm_capture_period_bytes_min, 
			(l2_dma_transfer_direction_t)BUF2MEM,1);
			}
		//#endif
	}
	else  //input stream has been stopped
	{
		printk("input stream stopped\n");
		clear_bit(1,&akpcm->captureStrmDMARunning);
		complete(&(akpcm->captureHWDMA_completion));

		/* FIXME */
		if (akpcm->ops->adc_exit)
			akpcm->ops->adc_exit(akpcm->dai);
	}
}

/**
 * @brief  trigger callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_playback_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	dma_addr_t paddr = runtime->dma_addr;
	void *vaddr = runtime->dma_area;	
	snd_pcm_uframes_t avail;	
	int avail_bytes, pend_bytes, period_bytes;
	int start_pos0, start_pos1;
	int bytespersample = frames_to_bytes(runtime, 1);
	
	void *vdst_addr0 = akpcm->pp_buf_addr[0];
	void *vdst_addr1 = akpcm->pp_buf_addr[1];
	void *pdst_addr0 = (void *)virt_to_phys(vdst_addr0);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	{
		u8 id = akpcm->L2BufID_For_DAC;

#if defined(AK_PCM_DELAY_CLOSE_DAC)
		del_timer(&akpcm->stopoutput_work_timer);
#endif
		set_bit(0,&playback_statu);  //set bit to inform that playback stream is running
		set_bit(1,&akpcm->playbackStrmDMARunning);   //set bit to inform that DMA is working
		init_completion(&(akpcm->playbackHWDMA_completion));

		if (akpcm->use_optimal_period) {
			avail = snd_pcm_playback_avail(runtime);
			avail_bytes  = avail * bytespersample;
			pend_bytes = (runtime->buffer_size - avail) * bytespersample;
			period_bytes = akpcm->optimal_period_bytes;
			start_pos0 = akpcm->PlaybackCurrPos;
			start_pos1 = akpcm->PlaybackCurrPos + period_bytes;

			if (pend_bytes >= 2 * period_bytes) {
				memcpy(vdst_addr0, vaddr + start_pos0, period_bytes);
				memcpy(vdst_addr1, vaddr + start_pos1, period_bytes);
			} else if (pend_bytes > period_bytes) {
				memcpy(vdst_addr0, vaddr + start_pos0, period_bytes);
				memcpy(vdst_addr1, vaddr + start_pos1, period_bytes);
				memset(vdst_addr1 + pend_bytes - period_bytes, 0, 2 * period_bytes - pend_bytes);
			} else {
				memcpy(vdst_addr0, vaddr + start_pos0, period_bytes);
				memset(vdst_addr0 + pend_bytes, 0, period_bytes - pend_bytes);
				memset(vdst_addr1, 0, period_bytes);
			}

			akpcm->playing_idx = 0;
			akpcm->PlaybackCurrPos += period_bytes;
			l2_clr_status(id);
			l2_combuf_dma((unsigned long)pdst_addr0, id, period_bytes, 
				(l2_dma_transfer_direction_t)MEM2BUF,1);//start dma
		}else {
			l2_clr_status(id);
			l2_combuf_dma(paddr + akpcm->PlaybackCurrPos, id, frames_to_bytes(runtime,runtime->period_size), 
				(l2_dma_transfer_direction_t)MEM2BUF,1);//start dma

		}

#ifdef CONFIG_PCM_DUMP
		if(akpcm->enableDump){
			unsigned char *pcmDump_dma_area=runtime->dma_area;
			if((akpcm->pcmDumpSize+akpcm_playback_period_bytes_min)<=akpcm_dump_data_size){
				//printk(KERN_ERR "akpcm->dumpsize=%d,akpcm->PlaybackCurrPos=%d \n",akpcm->dumpsize,akpcm->PlaybackCurrPos);
				memcpy(akpcm->pcmDumpDataBuffer+akpcm->pcmDumpSize,pcmDump_dma_area+akpcm->PlaybackCurrPos,akpcm_playback_period_bytes_min);
				akpcm->pcmDumpSize+=akpcm_playback_period_bytes_min;
			}else{
				printk(KERN_ERR "akpcm->enableDump false\n");
				akpcm->enableDump=false;
			}
		}
#endif
		return 0;
	}
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		clear_bit(0,&playback_statu); //stop playback stream
		return 0;
	}
	return -EINVAL;
}

/**
 * @brief  trigger callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	//struct snd_pcm_runtime *runtime = substream->runtime;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	{
		schedule_delayed_work(&akpcm->ds_work, msecs_to_jiffies(300));
		return 0;	
	}
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		clear_bit(0,&akpcm->captureStrmDMARunning); //stop capture stream
		return 0;		
	}
	return -EINVAL;
}

static void delay_start_work(struct work_struct *work) 
{
	struct snd_akpcm *akpcm = container_of(work, struct snd_akpcm, ds_work.work);
	struct snd_pcm_substream *substream = akpcm->capturesubstrm;
	struct snd_pcm_runtime *runtime = substream->runtime;

	dma_addr_t paddr = runtime->dma_addr;
	u8 id = akpcm->L2BufID_For_ADC23;
	set_bit(0,&akpcm->captureStrmDMARunning);  //set bit to inform that capture stream is running
	set_bit(1,&akpcm->captureStrmDMARunning);  //set bit to inform that DMA is working
	init_completion(&(akpcm->captureHWDMA_completion));
	l2_clr_status(id);		
	l2_combuf_dma(paddr, id, akpcm_capture_period_bytes_min, 
			(l2_dma_transfer_direction_t)BUF2MEM,1); //start dma

}

/**
 * @brief  prepare callback,open DAC, power on hp/speaker
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_playback_prepare(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int bytespersample = frames_to_bytes(runtime, 1);
	int last_period_bytes = akpcm->optimal_period_bytes;

	if(test_bit(1,&akpcm->playbackStrmDMARunning))
	{
		wait_for_completion(&(akpcm->playbackHWDMA_completion));
	}
	akpcm->PlaybackCurrPos = 0;

	if (((runtime->period_size % akpcm_playback_period_aligned) != 0) || 
		((runtime->buffer_size % runtime->period_size) != 0)) {

		/*Calculate the optimal period size depending on the aligned period size*/
		if (runtime->period_size % akpcm_playback_period_aligned == 0) {
			//Doesn't need changed the original period_size
			akpcm->optimal_period_size = runtime->period_size; 
		} else {
			akpcm->optimal_period_size = runtime->period_size + akpcm_playback_period_aligned
					- (runtime->period_size % akpcm_playback_period_aligned);
		}
		akpcm->optimal_period_bytes = akpcm->optimal_period_size * bytespersample;

		/* Allocate or reallocate contigous for Pingpong Buffer */
		if ((last_period_bytes != 0) && (last_period_bytes != akpcm->optimal_period_bytes)) {
			if(akpcm->pp_buf_addr[0])
				snd_free_pages(akpcm->pp_buf_addr[0], GFP_KERNEL);				

			akpcm->pp_buf_addr[0] = snd_malloc_pages(akpcm->optimal_period_bytes * 2, GFP_KERNEL);

			printk(KERN_ERR"Pingpong Buffer Warning: Reallocate memory[old=%d, new=%d]\n", 
					last_period_bytes, akpcm->optimal_period_bytes);
		} else {
			if (akpcm->pp_buf_addr[0] == NULL)
				akpcm->pp_buf_addr[0] = snd_malloc_pages(akpcm->optimal_period_bytes * 2, GFP_KERNEL);
		}

		/* Decide whether to start Pingpong Buffer */		
		if (akpcm->pp_buf_addr[0] == NULL) {
			printk(KERN_ERR"Pingpong Buffer Warning: allocate memory failed!\n");
			akpcm->optimal_period_size = 0;
			akpcm->optimal_period_bytes = 0;
			akpcm->use_optimal_period = false;
		} else {
			akpcm->pp_buf_addr[1] = akpcm->pp_buf_addr[0] + akpcm->optimal_period_bytes;
			akpcm->use_optimal_period = true;
		}
	} else {
		akpcm->optimal_period_size = 0;
		akpcm->optimal_period_bytes = 0;
		akpcm->use_optimal_period = false;
	}

	if (akpcm->use_optimal_period) {
		l2_set_dma_callback(akpcm->L2BufID_For_DAC, akpcm_playback_interrupt_optimize,(unsigned long)akpcm);
	}else
		l2_set_dma_callback(akpcm->L2BufID_For_DAC, akpcm_playback_interrupt,(unsigned long)akpcm);

	/* FIXME */
	if (akpcm->ops->start_to_play) {
		akpcm->ops->start_to_play(akpcm->dai, runtime->channels, runtime->rate);
	}

	return 0;
}

/**
 * @brief  prepare callback,open ADC23, power on mic
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_capture_prepare(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	l2_set_dma_callback(akpcm->L2BufID_For_ADC23,akpcm_capture_interrupt,(unsigned long)akpcm);
	do_gettimeofday(&capSync.tv);
	capSync.adcCapture_bytes = 0;
	capSync.frame_bits = runtime->frame_bits;
	capSync.rate = runtime->rate;
	
	akpcm->CaptureCurrPos = 0;
	/* FIXME */
	if (akpcm->ops->set_adc_samplerate)
		capSync.rate = akpcm->ops->set_adc_samplerate(akpcm->dai, runtime->rate);

	if (akpcm->ops->set_adc_channels)
		akpcm->ops->set_adc_channels(akpcm->dai, runtime->channels);

	if (akpcm->ops->adc_init)
		akpcm->ops->adc_init(akpcm->dai);

	if (akpcm->ops->capture_start)
		akpcm->ops->capture_start(akpcm->dai);


	return 0;
}

/**
 * @brief  pointer callback,updata ringbuffer pointer
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static snd_pcm_uframes_t akpcm_playback_pointer(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	return(bytes_to_frames(runtime,akpcm->PlaybackCurrPos)); //updata ringbuffer pointer	
}

/**
 * @brief  pointer callback, updata ringbuffer pointer
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static snd_pcm_uframes_t akpcm_capture_pointer(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	return(bytes_to_frames(runtime,akpcm->CaptureCurrPos)); //updata ringbuffer pointer
}

static struct snd_pcm_hardware akpcm_playback_hardware = {
	.info =			(SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_BLOCK_TRANSFER |
				 SNDRV_PCM_INFO_RESUME | 
				 SNDRV_PCM_INFO_PAUSE |
				 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		USE_FORMATS,
	.rates =		USE_RATE,
	.rate_min =		PLAYBACK_USE_RATE_MIN,
	.rate_max =		PLAYBACK_USE_RATE_MAX,
	.channels_min =		PLAYBACK_USE_CHANNELS_MIN,
	.channels_max =		PLAYBACK_USE_CHANNELS_MAX,
	.buffer_bytes_max =	akpcm_playback_buf_bytes_max,
	.period_bytes_min =	akpcm_playback_period_bytes_min,
	.period_bytes_max =	akpcm_playback_period_bytes_max,
	.periods_min =		akpcm_playback_periods_min,
	.periods_max =		akpcm_playback_periods_max,
	.fifo_size =		0,
};

static struct snd_pcm_hardware akpcm_capture_hardware = {
	.info =			(SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_BLOCK_TRANSFER |
				 SNDRV_PCM_INFO_RESUME |
				 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		USE_FORMATS,
	.rates =		USE_RATE,
	.rate_min =		CAPTURE_USE_RATE_MIN,
	.rate_max =		CAPTURE_USE_RATE_MAX,
	.channels_min =		CAPTURE_USE_CHANNELS_MIN,
	.channels_max =		CAPTURE_USE_CHANNELS_MAX,
	.buffer_bytes_max =	akpcm_capture_buf_bytes_max,
	.period_bytes_min =	akpcm_capture_period_bytes_min,
	.period_bytes_max =	akpcm_capture_period_bytes_min,
	.periods_min =		akpcm_capture_periods_min,
	.periods_max =		akpcm_capture_periods_max,
	.fifo_size =		0,
};

/**
 * @brief  hw_params callback, malloc ringbuffer
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_playback_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *hw_params)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	if(BUF_NULL==akpcm->L2BufID_For_DAC)
	{
		akpcm->L2BufID_For_DAC = l2_alloc((l2_device_t)ADDR_DAC); //alloc l2 buffer for DAC	
		if(BUF_NULL==akpcm->L2BufID_For_DAC)
		{
			printk(KERN_ERR "alloc L2 buffer for DAC error!");
			return -ENOMEM;
		}
	}
	return(snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params)));
}

/**
 * @brief  hw_params callback, malloc ringbuffer
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_capture_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *hw_params)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	if(BUF_NULL==akpcm->L2BufID_For_ADC23)
	{
		akpcm->L2BufID_For_ADC23 = l2_alloc((l2_device_t)ADDR_ADC); //alloc l2 buffer for ADC23	
		if(BUF_NULL==akpcm->L2BufID_For_ADC23)
		{
			printk(KERN_ERR "alloc L2 buffer for DAC error!");
			return -ENOMEM;
		}
	}
	return(snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params)));
}

/**
 * @brief  hw_free callback, free ringbuffer
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_playback_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	
	if(test_bit(1,&akpcm->playbackStrmDMARunning))
	{
		wait_for_completion(&(akpcm->playbackHWDMA_completion));
	}
	if(BUF_NULL!=akpcm->L2BufID_For_DAC)
	{
		/* FIXME */
#if !defined(AK_PCM_DELAY_CLOSE_DAC)		
		if (akpcm->ops->playback_end) {
			akpcm->ops->playback_end(akpcm->dai);
		}
#endif
		if (akpcm->ops->dac_exit) {
			akpcm->ops->dac_exit(akpcm->dai);
		}

		l2_free((l2_device_t)ADDR_DAC);
		akpcm->L2BufID_For_DAC = BUF_NULL;
	}
	 return snd_pcm_lib_free_pages(substream);
}

/**
 * @brief  hw_free callback, free ringbuffer
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_capture_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	
	if(test_bit(1,&akpcm->captureStrmDMARunning))
	{
		wait_for_completion(&(akpcm->captureHWDMA_completion));
	}
	if(BUF_NULL!=akpcm->L2BufID_For_ADC23)
	{
		/* FIXME */
		if (akpcm->ops->adc_exit)
			akpcm->ops->adc_exit(akpcm->dai);
		if (akpcm->ops->capture_end)
			akpcm->ops->capture_end(akpcm->dai);

		l2_free((l2_device_t)ADDR_ADC);
		akpcm->L2BufID_For_ADC23 = BUF_NULL;
	}
	return(snd_pcm_lib_free_pages(substream));	
}

/**
 * @brief  open callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_playback_open(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	akpcm->playbacksubstrm = substream;
	runtime->hw = akpcm_playback_hardware;
	dac_clock = 0;

	if (akpcm->pp_buf_addr[0]) {
		printk(KERN_ERR"akpcm_playback_open==>pp_buf_addr[0]=0x%08x, memory leak happened\n", (unsigned int)akpcm->pp_buf_addr[0]);
	} else {
		akpcm->pp_buf_addr[0] = NULL;
		akpcm->pp_buf_addr[1] = NULL;
	}
	akpcm->optimal_period_size = 0;
	akpcm->optimal_period_bytes = 0;

	//#ifdef CONFIG_SUPPORT_AEC
	if (!akpcm->playback_data) 
	{
		akpcm->playback_data = kmalloc(akpcm_playback_period_bytes_min * 2, GFP_KERNEL);
		if (!akpcm->playback_data)
		{
			printk("ak37pcm_playback_open==>allocate memory for playback_data failed!\n");
		}
	}
	//#endif
	return 0;
}

/**
 * @brief  open callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_capture_open(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	
	akpcm->capturesubstrm = substream;
	runtime->hw = akpcm_capture_hardware;	
	akpcm->CaptureCurrPos = 0;
	
//#ifdef CONFIG_SUPPORT_AEC
	if(!akpcm->capture_data)
	{
		akpcm->capture_data = kmalloc(akpcm_capture_period_bytes_min, GFP_KERNEL);
		if (!akpcm->capture_data)
		{
			printk("akpcm_capture_open==>allocate memory for capture_data failed!\n");
		}
	}
	if(!akpcm->temp)
	{
		akpcm->temp= kmalloc(akpcm_capture_period_bytes_min, GFP_KERNEL);
		if (!akpcm->temp)
		{
			printk("akpcm_capture_open==>allocate memory for temp failed!\n");
		}
		//memset(akpcm->temp, 0x)
	}
//#endif
	return 0;
}

/**
 * @brief  close callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_playback_close(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);

	akpcm->playbacksubstrm=NULL;
	dac_clock = 0;

	if (akpcm->pp_buf_addr[0] != NULL) {
		snd_free_pages(akpcm->pp_buf_addr[0], GFP_KERNEL);
		akpcm->pp_buf_addr[0] = NULL;
		akpcm->pp_buf_addr[1] = NULL;
	}

	//#ifdef CONFIG_SUPPORT_AEC
	if (akpcm->playback_data) 
	{
		kfree(akpcm->playback_data);
		akpcm->playback_data = NULL;
	}	
	//#endif
	
	return 0;
}

/**
 * @brief  close callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_capture_close(struct snd_pcm_substream *substream)
{
	struct snd_akpcm *akpcm = snd_pcm_substream_chip(substream);
	capSync.adcCapture_bytes = 0;
	akpcm->capturesubstrm=NULL;
	return 0;
}

/**
 * @brief  mmap callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int akpcm_pcm_mmap(struct snd_pcm_substream *substream,
	struct vm_area_struct *vma)
{
	return remap_pfn_range(vma, vma->vm_start,
		       substream->dma_buffer.addr >> PAGE_SHIFT,
		       vma->vm_end - vma->vm_start, vma->vm_page_prot);
}


static struct snd_pcm_ops akpcm_playback_ops = {
	.open =		akpcm_playback_open,
	.close =	akpcm_playback_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	akpcm_playback_hw_params,
	.hw_free =	akpcm_playback_hw_free,
	.prepare =	akpcm_playback_prepare,
	.trigger =	akpcm_playback_trigger,
	.pointer =	akpcm_playback_pointer,
	.mmap =	    akpcm_pcm_mmap,
};

static struct snd_pcm_ops akpcm_capture_ops = {
	.open =		akpcm_capture_open,
	.close =	akpcm_capture_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	akpcm_capture_hw_params,
	.hw_free =	akpcm_capture_hw_free,
	.prepare =	akpcm_capture_prepare,
	.trigger =	akpcm_capture_trigger,
	.pointer =	akpcm_capture_pointer,
	.mmap =	    akpcm_pcm_mmap,
};


#ifdef CONFIG_CPU_FREQ

#define freq_to_akpcm(_n) container_of(_n, struct snd_akpcm, freq_transition)

/**
 * @brief     handle cpu frequency changing
 *            Note: need to handle in PLL changed mode only
 * @author    Cao LianMing
 * @date      2011-07-26
 * @parm      [in]  nb :  the nitifier_block data struct
 * @parm      [in]  val :  CPUFREQ_PRECHANGE / CPUFREQ_POSTCHANGE
 * @parm      [in]  data :  argument
 * @return    int :  if successful return 0, otherwise return nagative
 * @retval     0 : handle successful
 * @retval    <0 : handle failed
 */
static int akpcm_cpufreq_transition(struct notifier_block *nb,
				      unsigned long val, void *data)
{
	struct snd_akpcm *akpcm = freq_to_akpcm(nb);
	struct cpufreq_freqs *freqs = data;	
	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;

	if (val == CPUFREQ_PRECHANGE) {
		if (freqs->old_cpufreq.pll_sel != freqs->new_cpufreq.pll_sel) {
			if (test_bit(0,&playback_statu)) {
				printk("---- stop pcm playback\n");

				substream = akpcm->playbacksubstrm;
				/* suspend PCM data transmission */
				akpcm_playback_trigger(substream, SNDRV_PCM_TRIGGER_SUSPEND);
				set_bit(2, &akpcm->playbackStrmDMARunning);
				wait_for_completion(&(akpcm->playbackHWDMA_completion));
			} else {
				clear_bit(2, &akpcm->playbackStrmDMARunning);
			}
		}
	} else if (val == CPUFREQ_POSTCHANGE) {
		if (freqs->old_cpufreq.pll_sel != freqs->new_cpufreq.pll_sel) {
			if (test_bit(2,&akpcm->playbackStrmDMARunning)) {
				printk("---- restart pcm playback\n");

				substream = akpcm->playbacksubstrm;
				runtime = substream->runtime;
				/* reconfig sample rate after PLL has been changed */
				/* FIXME */
				if (akpcm->ops->set_dac_samplerate)
					akpcm->ops->set_dac_samplerate(akpcm->dai, runtime->rate);

				/* resume PCM data transmission */
				akpcm_playback_trigger(substream, SNDRV_PCM_TRIGGER_RESUME);
				clear_bit(2, &akpcm->playbackStrmDMARunning);
			}
		}
	}

	return 0;
}

/**
 * @brief     register the akpcm cpufreq changing handle function to the cpufreq core
 * @author    Cao LianMing
 * @date      2011-07-26
 * @parm      [in]  akpcm :  the private data struct of the akpcm driver
 * @return    int :  if register successful return 0, otherwise, return negative
 * @retval     0 :  successful
 * @retval    <0 :  failed
 */
static inline int akpcm_cpufreq_register(struct snd_akpcm *akpcm)
{
	akpcm->freq_transition.notifier_call = akpcm_cpufreq_transition;

	return cpufreq_register_notifier(&akpcm->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
}

/**
 * @brief     unregister the akpcm cpufreq changing handle function from the cpufreq core
 * @author    Cao LianMing
 * @date      2011-07-26
 * @parm      [in]  akpcm :  the private data struct of the akpcm driver
 * @return    int :  if unregister successful return 0, otherwise, return negative
 * @retval     0 :  successful
 * @retval    <0 :  failed
 */
static inline void akpcm_cpufreq_deregister(struct snd_akpcm *akpcm)
{
	cpufreq_unregister_notifier(&akpcm->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}

#else
static inline int akpcm_cpufreq_register(struct snd_akpcm *akpcm)
{
	return 0;
}

static inline void akpcm_cpufreq_deregister(struct snd_akpcm *akpcm)
{
}
#endif

/**
 * @brief     Shutdown all ak ad/da modules
 * @author    Cheng JunYi
 * @date      2011-09-27
 * @parm      [in]  in_akpcm :  the private data struct of akpcm driver
 * @return    void
 * @retval
 */
static inline void akpcm_close(struct snd_akpcm *in_akpcm)
{

	struct snd_akpcm *akpcm = in_akpcm;
	if(test_bit(1,&akpcm->playbackStrmDMARunning))
	{
		wait_for_completion(&(akpcm->playbackHWDMA_completion));
	}
	
	if(test_bit(1,&akpcm->captureStrmDMARunning))
	{
		wait_for_completion(&(akpcm->captureHWDMA_completion));
	}

	//close analog module
	/* FIXME */
	if (akpcm->ops->dac_exit)
		akpcm->ops->dac_exit(akpcm->dai);
	if (akpcm->ops->playback_end)
		akpcm->ops->playback_end(akpcm->dai);
	if (akpcm->ops->adc_exit)
		akpcm->ops->adc_exit(akpcm->dai);
	if (akpcm->ops->capture_end)
		akpcm->ops->capture_end(akpcm->dai);
}


/**
 * @brief  create new card
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int __devinit snd_card_akpcm_pcm(struct snd_akpcm *akpcm, int device,
					int substreams)
{
	struct snd_pcm *pcm;
	struct snd_pcm_substream *substream;
	int err;
	err = snd_pcm_new(akpcm->card, "akpcm PCM", device,
			       substreams, substreams, &pcm);
	if (err < 0)
		return err;
	akpcm->pcm = pcm;
	cur_pcm = pcm;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &akpcm_playback_ops); //register callbacks
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &akpcm_capture_ops);//register callbacks
	pcm->private_data = akpcm;
	pcm->info_flags = 0;
	strcpy(pcm->name, "akpcm PCM");
	
	substream = pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream;
	snd_pcm_lib_preallocate_pages(substream, SNDRV_DMA_TYPE_DEV, akpcm->card->dev, 
			akpcm_playback_buf_bytes_max, akpcm_playback_buf_bytes_max);

	substream = pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
	snd_pcm_lib_preallocate_pages(substream, SNDRV_DMA_TYPE_DEV, akpcm->card->dev, 
			akpcm_capture_buf_bytes_max, akpcm_capture_buf_bytes_max);
	return 0;
}

/**************
 * proc interface
 **************/
#ifdef CONFIG_PCM_DUMP

/**
 * @brief     Dump the ak ad/da registers to the proc file
 * @author    Cheng JunYi
 * @date      2011-08-04
 * @parm      [in]  entry :  snd handle of akpcm driver
 * @parm      [in]  buffer :  buffer of the proc file
 * @return    void
 * @retval
 */
static void akpcm_registers_read(struct snd_info_entry *entry, struct snd_info_buffer *buffer)
{

	//snd_iprintf(buffer,"ADC23 clock Register2          =0x%lu\n",REG32(RegAddr.pAddress0800 + CLK_DIV_REG2));
	
	//snd_iprintf(buffer,"Multi-func Control Register1   =0x%lu\n",REG32(RegAddr.pAddress0800 + MULTIPLE_FUN_CTRL_REG1));
		
	//snd_iprintf(buffer,"Analog Control Register1       =0x%lu\n",REG32(RegAddr.pAddress0800 + ANALOG_CTRL_REG1_READ));
	//snd_iprintf(buffer,"Analog Control Register2       =0x%lu\n",REG32(RegAddr.pAddress0800 + ANALOG_CTRL_REG2_READ));
	//snd_iprintf(buffer,"Analog Control Register3       =0x%lu\n",REG32(RegAddr.pAddress0800 + ANALOG_CTRL_REG3));
	//snd_iprintf(buffer,"Analog Control Register4       =0x%lu\n",REG32(RegAddr.pAddress0800 + ANALOG_CTRL_REG4));

	//snd_iprintf(buffer,"ADC2 configuration Register    =0x%lu\n",REG32(RegAddr.pAddress2002D + ADC2MODE_CFG_REG));
	//snd_iprintf(buffer,"ADC2 Data Register             =0x%lu\n",REG32(RegAddr.pAddress2002D + I2S_CONFIG_REG));
	
	//snd_iprintf(buffer,"DAC configuration Register     =0x%lu\n",REG32(RegAddr.pAddress2002E + DAC_CONFIG_REG));
	//snd_iprintf(buffer,"CPU Data Register              =0x%lu\n",REG32(RegAddr.pAddress2002E + CPU_DATA_REG));
	
}


/**
 * @brief     Handle the write operation of proc file /proc/asound/card0/pcm-dumpctrl
 *            Start or stop dump data to the pcm-dumpdata file
 * @author    Cheng JunYi
 * @date      2011-08-04
 * @parm      [in]  entry :  snd handle of akpcm driver
 * @parm      [in]  buffer :  "open" means start dump pcm data to /proc/asound/card0/pcm-dumpdata;
                              "close" means stop dump pcm data to /proc/asound/card0/pcm-dumpdata
 * @return    void
 * @retval
 */
void akpcm_dumpctrl_write(struct snd_info_entry *entry,
		  struct snd_info_buffer *buffer)
{
	char line[256];
	char str[256];
	struct snd_akpcm *akpcm=entry->private_data;
	snd_info_get_line(buffer, line, 256);
	snd_info_get_str(str, line, 256);

	if(strncmp("open", str, strlen("open"))==0){
		
		memset(akpcm->pcmDumpDataBuffer,0,akpcm_dump_data_size);
		akpcm->pcmDumpSize=0;
		akpcm->enableDump=true;
		do_gettimeofday(&akpcm->pcmDumpTime);
		printk(KERN_ERR "akpcm->enableDump true\n");

	}else if(strncmp("close", str, strlen("close"))==0){
	
		akpcm->enableDump=false;
		akpcm->pcmDumpSize=0;
		printk(KERN_ERR "akpcm->enableDump false\n");

	}
}


/**
 * @brief     Handle the read operation of proc file /proc/asound/card0/pcm-dumpctrl
 *            Show the last dump time
 * @author    Cheng JunYi
 * @date      2011-08-04
 * @parm      [in]  entry :  snd handle of akpcm driver
 * @parm      [in]  buffer :  buffer of the proc file, will return the last dump time
 * @return    void
 * @retval
 */
static void akpcm_dumpctrl_read(struct snd_info_entry *entry, struct snd_info_buffer *buffer)
{
	
	struct snd_akpcm *akpcm=entry->private_data;
	struct tm  tm_result;

	time_to_tm(akpcm->pcmDumpTime.tv_sec,0,&tm_result);
	snd_iprintf(buffer, "last dump timestamp: %lu-%02d-%02d %02d:%02d:%02d UTC\n",tm_result.tm_year + 1900, tm_result.tm_mon + 1, tm_result.tm_mday, tm_result.tm_hour,tm_result.tm_min,tm_result.tm_sec);
	
}


/**
 * @brief     Handle the read operation of proc file /proc/asound/card0/pcm-dumpdata
 *            Return the pcm data to application
 * @author    Cheng JunYi
 * @date      2011-08-04
 * @parm      [in]  entry :  snd handle of akpcm driver
 * @parm      [in]  buffer :  buffer of the proc file, will return the pcm data
 * @return    void
 * @retval
 */
static long akpcm_dumpdata_read(struct snd_info_entry *entry,
				void *file_private_data,
				struct file *file, char __user *buf,
				unsigned long count, unsigned long pos)
{
	
	long size;
	
	struct snd_akpcm *akpcm=entry->private_data;

	size = count;
	if (pos + size > akpcm_dump_data_size)
		size = akpcm_dump_data_size - pos;
		
	if (size > 0) {
		if (copy_to_user(buf,akpcm->pcmDumpDataBuffer+pos,size))
			return -EFAULT;
	}
			
	return size;
}


static struct snd_info_entry_ops akpcm_dumpdata_proc_ops = {
	.read = akpcm_dumpdata_read,
};

#endif


/**
 * @brief     Handle the reboot event
 * @author    Cheng JunYi
 * @date      2011-09-27
 * @parm      [in]  nb :  the nitifier_block data struct
 * @parm      [in]  code :  message
 * @parm      [in]  unused :  argument, unused
 * @return    int :  handle successful or not
 * @retval    NOTIFY_DONE :  handle successful
 */
static int akpcm_reboot_notify(struct notifier_block *nb,
			       unsigned long code, void *unused)
{
	struct snd_akpcm *akpcm;

	printk(KERN_ERR "akpcm_reboot_notify \n");

	mutex_lock(&reboot_lock);

	if (!reboot_info)
		goto out;
	
	akpcm = reboot_info;
	akpcm_close(akpcm);

 out:
 	printk(KERN_ERR "akpcm_reboot_notify out\n");
	mutex_unlock(&reboot_lock);
	
	return NOTIFY_DONE;
}

static struct notifier_block akpcm_reboot_notifier = {
	.notifier_call = akpcm_reboot_notify,
};


//#ifdef CONFIG_SUPPORT_AEC
T_pVOID akpcm_capture_aec_kmalloc(T_U32 size)
{
	return kmalloc(size, GFP_KERNEL | GFP_ATOMIC);
}
T_VOID  akpcm_capture_aec_kfree(T_pVOID mem)
{
	kfree(mem);
}
//#endif

/**
 * @brief     Init the device which was probed, and register a snd device
 * @author    Cheng MingJuan
 * @date      2010-11-20
 * @parm      [in]  pdev :  the device definition
 * @return    int :  if init successful return 0, otherwise, return negative
 * @retval     0 :  successful
 * @retval    <0 :  failed
 */
static int __devinit snd_akpcm_probe(struct platform_device *devptr)
{
	struct snd_card *card;
	struct snd_akpcm *akpcm;
	//struct snd_info_entry *entry;
	int dev_id, err;

	memset(&capSync, 0, sizeof(capSync));
	//get analog control registers

	dev_id = devptr->id;
	if (dev_id < 0)
		dev_id = 0;
	err = snd_card_create(dev_id, NULL, THIS_MODULE, sizeof(struct snd_akpcm), &card);
	if (err < 0)
		return err;

	snd_card_set_dev(card, &devptr->dev);

	akpcm = card->private_data;
	akpcm->card = card;
	cur_card = card;
	
	//init l2 buf for audio
	akpcm->L2BufID_For_DAC = BUF_NULL;
	akpcm->L2BufID_For_ADC23 = BUF_NULL;

	err = snd_card_akpcm_pcm(akpcm, 0, 1);
	if (err < 0)
		goto __out_free_card;

	//init tiimer to stop output channel
	init_timer(&akpcm->stopoutput_work_timer);
	akpcm->stopoutput_work_timer.function = ak_close_dac_timer;
	akpcm->stopoutput_work_timer.data = (unsigned long)akpcm;

	akpcm->fetch_tasklet.func = alsabuf_to_ppbuf;
	akpcm->fetch_tasklet.data = (unsigned long)akpcm;
	akpcm->pp_buf_addr[0] = NULL;

	akpcm->close_dac.func = dac_exit_tasklet;
	akpcm->close_dac.data = (unsigned long)akpcm;


//#ifdef CONFIG_SUPPORT_AEC
		akpcm->capture_aec_tasklet.func = ak37pcm_capture_aec;
		akpcm->capture_aec_tasklet.data = (unsigned long)akpcm;
	
		akpcm->playback_aec_tasklet.func = ak37pcm_playback_aec;
		akpcm->playback_aec_tasklet.data = (unsigned long)akpcm;
//#endif

	INIT_DELAYED_WORK(&akpcm->ds_work, delay_start_work);	

	clear_bit(0,&playback_statu);
	clear_bit(1,&akpcm->playbackStrmDMARunning);
	clear_bit(0,&akpcm->captureStrmDMARunning);
	clear_bit(1,&akpcm->captureStrmDMARunning);

#ifdef CONFIG_PCM_DUMP
	
	snd_card_proc_new (akpcm->card, "registers", &entry);
	snd_info_set_text_ops(entry, akpcm, akpcm_registers_read);

	akpcm->pcmDumpDataBuffer = vmalloc(akpcm_dump_data_size);
	if (!akpcm->pcmDumpDataBuffer)
		return -ENOMEM;

	memset(akpcm->pcmDumpDataBuffer, 0, akpcm_dump_data_size);
	
	snd_card_proc_new (akpcm->card, "pcm-dumpctrl", &entry);
	entry->private_data = akpcm;
	entry->mode = S_IFREG | S_IRUGO | S_IWUSR;
	entry->c.text.write = akpcm_dumpctrl_write;
	snd_info_set_text_ops(entry, akpcm, akpcm_dumpctrl_read);

	if (! snd_card_proc_new(akpcm->card, "pcm-dumpdata", &entry)) {
		entry->content = SNDRV_INFO_CONTENT_DATA;
		entry->private_data = akpcm;
		entry->c.ops = &akpcm_dumpdata_proc_ops;
		entry->size = akpcm_dump_data_size;
		entry->mode = S_IFREG|S_IRUGO;
	}
	
#endif

	strcpy(card->driver, "akpcm");
	strcpy(card->shortname, "Ak AD/DA");
	sprintf(card->longname, "Ak ADC DAC pcm input & output module %i", dev_id + 1);

	err = akpcm_cpufreq_register(akpcm);
	if (err < 0) {
		printk(KERN_ERR "Failed to register cpufreq\n");
	}

	mutex_lock(&reboot_lock);
	reboot_info = akpcm;
	mutex_unlock(&reboot_lock);

//#ifdef CONFIG_SUPPORT_AEC
		memset(&akpcm->p_aecin, 0, sizeof(akpcm->p_aecin));
		memset(&akpcm->p_aecbufs, 0, sizeof(akpcm->p_aecbufs));
		akpcm->p_aecin.cb_fun.Malloc = akpcm_capture_aec_kmalloc;
		akpcm->p_aecin.cb_fun.Free = akpcm_capture_aec_kfree;
		akpcm->p_aecin.cb_fun.printf = (AEC_CALLBACK_FUN_PRINTF)printk;
		//akpcm->p_aecin.m_info.m_Type = AEC_TYPE_2;
		//akpcm->p_aecin.m_info.m_BitsPerSample = AEC_BITSPERSAMPLE;
		akpcm->p_aecin.m_info.m_Channels = AEC_CHANNELS;
		akpcm->p_aecin.m_info.m_SampleRate = AEC_SAMPLERATE;
		
		//akpcm->p_aecin.m_info.m_Private.m_aec.m_PreprocessEna = 0;
		//akpcm->p_aecin.m_info.m_Private.m_aec.m_framelen = AEC_NN;
		akpcm->p_aecin.m_info.m_Private.m_aec.m_tail = 1280;
		akpcm->p_aecin.m_info.m_Private.m_aec.m_aecBypass = 0;
		akpcm->p_aecin.m_info.m_Private.m_aec.m_framelen = 256;
    	akpcm->p_aecin.m_info.m_Private.m_aec.m_PreprocessEna = 1;
		akpcm->p_aecin.m_info.m_Private.m_aec.AGClevel = 24576;
		akpcm->p_aecin.m_info.m_Private.m_aec.maxGain = 3;
		akpcm->p_aecin.m_info.m_Private.m_aec.DacVolume = 1024;
		akpcm->p_aecin.m_info.m_Private.m_aec.AdcCutTime = 100;
		akpcm->p_aecin.m_info.m_Private.m_aec.AdcMinSpeechPow = 1024;
		akpcm->p_aecin.m_info.m_Private.m_aec.DacMinSpeechPow = 512;
		akpcm->p_aecin.m_info.m_Private.m_aec.AdcSpeechMultiple = (T_U32)(1.8*(1<<14));
		akpcm->p_aecin.m_info.m_Private.m_aec.DacSpeechMultiple = (T_U32)(1.8*(1<<14));
		akpcm->p_aecin.m_info.m_Private.m_aec.AdcSpeechHoldTime = 900;
		akpcm->p_aecin.m_info.m_Private.m_aec.DacSpeechHoldTime = 900;
		akpcm->p_aecin.m_info.m_Private.m_aec.AdcConvergTime = 10000;
		akpcm->p_aecin.m_info.m_Private.m_aec.DacConvergTime = 10000;
		akpcm->pfilter = AECLib_Open(&akpcm->p_aecin);
		if (akpcm->pfilter) {
			printk("AEC function has been successfully activated!\n");
		} else {
			printk("Failed to initialize AEC Lib, AEC function is closed\n");
		}
//#endif



	err = snd_card_register(card);
	if (err == 0) {
		platform_set_drvdata(devptr, card);
		return 0;
	}

__out_free_card:
	snd_card_free(card);
	
	return err;
}

/**
 * @brief     De-init the device which will be removed, and unregister the snd device
 * @author    Cheng MingJuan
 * @date      2010-11-22
 * @parm      [in]  pdev :  the device definition
 * @return    int :  if handle successful return 0, otherwise, return negative
 * @retval     0 :  successful
 * @retval    <0 :  failed
 */
static int __devexit snd_akpcm_remove(struct platform_device *devptr)
{
	struct snd_card *card = platform_get_drvdata(devptr);
	struct snd_akpcm *akpcm = card->private_data;

	akpcm_close(akpcm);

	akpcm_cpufreq_deregister(akpcm);

	del_timer(&akpcm->stopoutput_work_timer);

//#ifdef CONFIG_SUPPORT_AEC
	AECLib_Close(akpcm->pfilter);
	kfree(akpcm->capture_data);
	kfree(akpcm->playback_data);
//#endif
	/* FIXME */
	snd_card_set_dev(card, NULL);
	snd_card_free(card);
	platform_set_drvdata(devptr, NULL);
	return 0;
}

#ifdef CONFIG_PM
/**
 * @brief  suspend callback
 * @author  Cheng Mingjuan
 * @revisor  Wu Daochao(2012-09-24)
 * @date   
 * @return void
 */
static int snd_ak_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct snd_card *card = platform_get_drvdata(pdev);
	struct snd_akpcm *akpcm = card->private_data;

	/* FIXME */
	akpcm_close(akpcm);

	return 0;
}

/**
 * @brief  resume callback
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int snd_ak_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define snd_ak_suspend NULL
#define snd_ak_resume NULL
#endif


#define SND_AKPCM_DRIVER	"snd_akpcm"

static struct platform_driver snd_akpcm_driver = {
	.probe		= snd_akpcm_probe,
	.remove		= __devexit_p(snd_akpcm_remove),
	.suspend	= snd_ak_suspend,
	.resume		= snd_ak_resume,
	.driver		= {
		.name	= SND_AKPCM_DRIVER
	},
};

/**
 * @brief  register driver
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static int __init alsa_card_akpcm_init(void)
{
	int err;

	err = platform_driver_register(&snd_akpcm_driver);
	if (err < 0)
		return err;

	register_reboot_notifier(&akpcm_reboot_notifier);
	
	return 0;
}

/**
 * @brief  unregister driver
 * @author  Cheng Mingjuan
 * @date   
 * @return void
 */
static void __exit alsa_card_akpcm_exit(void)
{
	platform_driver_unregister(&snd_akpcm_driver);
	unregister_reboot_notifier(&akpcm_reboot_notifier);
}

module_init(alsa_card_akpcm_init);
module_exit(alsa_card_akpcm_exit);

MODULE_AUTHOR("Anyka, Inc.");
MODULE_DESCRIPTION("akpcm soundcard");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA,akpcm soundcard}}");

