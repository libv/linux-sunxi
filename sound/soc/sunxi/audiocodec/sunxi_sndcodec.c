/*
 * sound\soc\sunxi\audiocodec\sunxi_sndcodec.c
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@Reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include <linux/io.h>

static struct snd_soc_dai_link sunxi_sndpcm_dai_link = {
	.name 			= "audiocodec",
	.stream_name 	= "SUNXI-CODEC",
	.cpu_dai_name 	= "sunxi-codec",
	.codec_dai_name = "sndcodec",
	.platform_name 	= "sunxi-pcm-codec-audio",
	.codec_name 	= "sunxi-pcm-codec",
};

static struct snd_soc_card snd_soc_sunxi_sndpcm = {
	.name 		= "audiocodec",
	.owner 		= THIS_MODULE,
	.dai_link 	= &sunxi_sndpcm_dai_link,
	.num_links 	= 1,
};

static struct platform_device *sunxi_sndpcm_device;

static int __init sunxi_sndpcm_init(void)
{
	int ret = 0;

	sunxi_sndpcm_device = platform_device_alloc("soc-audio", 0);
	if(!sunxi_sndpcm_device)
		return -ENOMEM;
	platform_set_drvdata(sunxi_sndpcm_device, &snd_soc_sunxi_sndpcm);
	ret = platform_device_add(sunxi_sndpcm_device);		
	if (ret) {			
		platform_device_put(sunxi_sndpcm_device);
	}

	return ret;
}

static void __exit sunxi_sndpcm_exit(void)
{
	platform_device_unregister(sunxi_sndpcm_device);
}

module_init(sunxi_sndpcm_init);
module_exit(sunxi_sndpcm_exit);

MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("SUNXI_sndpcm ALSA SoC audio driver");
MODULE_LICENSE("GPL");
